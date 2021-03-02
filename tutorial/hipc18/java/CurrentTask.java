
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ThreadFactory;

class CurrentTask
{
  List<CheckedCompletableFuture> owned;
  // Count of the number of promises in owned which no longer
  // belong to us. (!Config.GC_OWNED_LIST)
  int defunctPromises = 0;
  volatile CheckedCompletableFuture waitingOn = null;

  CurrentTask (List<CheckedCompletableFuture> initialOwned)
  {
    if (Config.VALIDATE_OWNERSHIP)
      owned = initialOwned;
    else
      owned = null;
  }

  void gcOwned ()
  {
    owned.removeIf((CheckedCompletableFuture p) -> p.owner != this);
    defunctPromises = 0;
  }

  static class ThreadWithStorage extends Thread
  {
    private CurrentTask currentTask = null;

    ThreadWithStorage (Runnable r)
    {
      super(r);
    }
  }

  static class ThreadWithStorageFactory implements ThreadFactory
  {
    @Override
    public Thread newThread (Runnable r)
    {
      return new ThreadWithStorage(r);
    }
  }

  static CurrentTask get ()
  {
    if (!Config.VALIDATE_OWNERSHIP)
      return null;

    Thread t = Thread.currentThread();
    if (t instanceof ThreadWithStorage)
      return ((ThreadWithStorage) t).currentTask;
    return null;
  }

  static void set (CurrentTask t)
  {
    ((ThreadWithStorage) Thread.currentThread()).currentTask = t;
  }
}
