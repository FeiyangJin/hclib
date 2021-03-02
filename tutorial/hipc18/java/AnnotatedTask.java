
import java.util.List;
import java.util.function.Supplier;

class AnnotatedTask<T> implements Runnable
{
  final CurrentTask task;
  CheckedCompletableFuture<T> result;
  final Runnable runnable;
  final Supplier<T> supplier;

  private AnnotatedTask (List<CheckedCompletableFuture> movingPromises,
                         Runnable r, Supplier<T> s)
  {
    task = new CurrentTask(movingPromises);
    result = null; // Set in OwnershipTrackingExecutor::execute
    runnable = r;
    supplier = s;
  }

  public static class PromiseAnnotation
  {
    List<CheckedCompletableFuture> promises;

    private PromiseAnnotation (List<CheckedCompletableFuture> promises)
    {
      this.promises = promises;
    }

    public <T> CheckedCompletableFuture<T> submit (Supplier<T> s)
    {
      AnnotatedTask<T> t = new AnnotatedTask<>(promises, s);
      OwnershipTrackingExecutor.defaultInstance.execute(t);
      return t.result;
    }

    public CheckedCompletableFuture<Void> submit (Runnable r)
    {
      AnnotatedTask<Void> t = new AnnotatedTask<>(promises, r);
      OwnershipTrackingExecutor.defaultInstance.execute(t);
      return t.result;
    }
  }

  public static PromiseAnnotation async (IPromiseCollection... movingPromises)
  {
    return new PromiseAnnotation(IPromiseCollection.merge(movingPromises));
  }

  AnnotatedTask (List<CheckedCompletableFuture> movingPromises, Supplier<T> s)
  {
    this(movingPromises, null, s);
  }

  AnnotatedTask (List<CheckedCompletableFuture> movingPromises, Runnable r)
  {
    this(movingPromises, r, null);
  }

  @Override
  public void run ()
  {
    if (runnable != null) {
      runnable.run();
      result.complete(null);
    } else {
      result.complete(supplier.get());
    }
  }
}

