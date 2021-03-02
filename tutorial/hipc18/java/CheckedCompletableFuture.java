
import java.util.Collections;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.function.Supplier;

class CheckedCompletableFuture<T> extends CompletableFuture<T> implements IPromiseCollection
{
  volatile CurrentTask owner;

  CheckedCompletableFuture (CurrentTask ownerOverride)
  {
    if (Test.COLLECT_STATS)
      Test.promiseCount.incrementAndGet();
    if (Config.VALIDATE_OWNERSHIP) {
      owner = ownerOverride;
      List<CheckedCompletableFuture> owned = owner.owned;
      if (owner.defunctPromises > 0) {
        // Replace a defunct promise in the list, instead of appending.
        for (int i = 0; i < owned.size(); i++) {
          if (owned.get(i).owner != owner) {
            owned.set(i, this);
            owner.defunctPromises--;
            break;
          }
        }
      }
      else {
        owned.add(this);
      }
    }
  }

  CheckedCompletableFuture ()
  {
    this(CurrentTask.get());
  }

  @Override
  public List<CheckedCompletableFuture> getPromises ()
  {
    return Collections.singletonList(this);
  }

  @Override
  public boolean complete (T value)
  {
    if (Config.VALIDATE_OWNERSHIP) {
      if (CurrentTask.get() != owner)
        throw new PromiseException("A task completed a promise it does not own");
      CurrentTask oldOwner = owner;
      owner = null;
      oldOwner.defunctPromises++;
      if (oldOwner.defunctPromises > Config.GC_OWNED_LIST_THRESHOLD)
        oldOwner.gcOwned();
    }
    return super.complete(value);
  }

  @Override
  public T get () throws InterruptedException, ExecutionException
  {
    if (Test.COLLECT_STATS)
      Test.waitCount.incrementAndGet();

    if (!Config.DETECT_DEADLOCKS)
      return super.get();

    try {
      check_await();
      return super.get();
    } catch (PromiseException e) {
      throw new CompletionException(e);
    } finally {
      CurrentTask.get().waitingOn = null;
    }
  }

  @Override
  public T join ()
  {
    if (Test.COLLECT_STATS)
      Test.waitCount.incrementAndGet();

    if (!Config.DETECT_DEADLOCKS)
      return super.join();

    try {
      check_await();
      return super.join();
    } catch (PromiseException e) {
      throw new CompletionException(e);
    } finally {
      CurrentTask.get().waitingOn = null;
    }
  }

  void check_await () throws PromiseException
  {
    CurrentTask t0 = CurrentTask.get();
    CurrentTask t = t0;
    t.waitingOn = this;
    CheckedCompletableFuture p = this;
    CurrentTask tnext = p.owner;
    while (tnext != t0) {
      if (tnext == null)
        break;
      CheckedCompletableFuture pnext = tnext.waitingOn;
      if (pnext == null)
        break;
      if (tnext != p.owner)
        break;
      t = tnext;
      p = pnext;
      tnext = p.owner;
    }
    if (tnext == t0)
      throw new PromiseException("Deadlocking await has occurred");
  }

  public static CheckedCompletableFuture<Void> runAsync (
    Runnable runnable)
  {
    return runAsync(NONE, runnable);
  }

  public static <U> CheckedCompletableFuture<U> supplyAsync (
    Supplier<U> supplier)
  {
    return supplyAsync(NONE, supplier);
  }

  public static CheckedCompletableFuture<Void> runAsync (
    IPromiseCollection pc, Runnable runnable)
  {
    return AnnotatedTask.async(pc).submit(runnable);
  }

  public static <U> CheckedCompletableFuture<U> supplyAsync (
    IPromiseCollection pc, Supplier<U> supplier)
  {
    return AnnotatedTask.async(pc).submit(supplier);
  }
}
