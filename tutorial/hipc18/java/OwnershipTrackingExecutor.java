
import java.util.ArrayList;
import java.util.concurrent.Executor;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

class OwnershipTrackingExecutor implements Executor
{
  static final OwnershipTrackingExecutor defaultInstance =
    new OwnershipTrackingExecutor();

  static class ExecutorImpl extends ThreadPoolExecutor
  {
    ExecutorImpl ()
    {
      super(Config.DEFAULT_EXECUTOR ? 0 : 625, Integer.MAX_VALUE, 60, TimeUnit.SECONDS,
            Config.DEFAULT_EXECUTOR ? new SynchronousQueue<Runnable>() : new LinkedBlockingQueue<Runnable>(),
            new CurrentTask.ThreadWithStorageFactory());
    }

    @Override
    protected void beforeExecute (Thread t, Runnable r)
    {
      if (!Config.VALIDATE_OWNERSHIP)
        return;
      CurrentTask.set(((AnnotatedTask) r).task);
    }

    @Override
    protected void afterExecute (Runnable r, Throwable t)
    {
      if (!Config.VALIDATE_OWNERSHIP)
        return;
      CurrentTask task = CurrentTask.get();
      for (CheckedCompletableFuture p : task.owned) {
        if (p.owner != task)
          continue;
        p.completeExceptionally(
          new PromiseException(
            "A task terminated without completing all promises", t));
      }
      task.owned = null;
    }
  }

  final ExecutorImpl impl = new ExecutorImpl();

  @Override
  public void execute (Runnable command)
  {
    if (Test.COLLECT_STATS)
      Test.taskCount.incrementAndGet();

    AnnotatedTask r = (AnnotatedTask) command;

    if (!Config.VALIDATE_OWNERSHIP) {
      r.result = new CheckedCompletableFuture<>(r.task);
      impl.execute(command);
      return;
    }

    CurrentTask parentTask = CurrentTask.get();
    for (CheckedCompletableFuture p : r.task.owned) {
      if (p.owner != parentTask)
        throw new PromiseException(
          "Current task does not own a promise to be moved");
      p.owner = r.task;
    }
    if (parentTask != null) {
      parentTask.defunctPromises += r.task.owned.size();
      if (parentTask.defunctPromises > Config.GC_OWNED_LIST_THRESHOLD)
        parentTask.gcOwned();
    }

    r.result = new CheckedCompletableFuture<>(r.task);
    impl.execute(r);
  }

  public void shutdown ()
  {
    impl.shutdown();
  }
}
