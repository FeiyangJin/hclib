
import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;

public class Channel <T> implements IPromiseCollection
{
  static class Payload <T>
  {
    final T value;
    final CheckedCompletableFuture<Payload<T>> next =
      new CheckedCompletableFuture<>();

    private Payload (T v)
    {
      value = v;
    }
  }

  private CheckedCompletableFuture<Payload<T>> latest =
    new CheckedCompletableFuture<>();

  @Override
  public List<CheckedCompletableFuture> getPromises ()
  {
    if (latest == null)
      return new ArrayList<>();

    return latest.getPromises(); // return a list with single item
  }

  public void put (T value)
  {
    Payload<T> p = new Payload<>(value);
    latest.complete(p);
    latest = p.next;
  }

  public void terminate ()
  {
    latest.complete(null);
    latest = null;
  }

  public ChannelReader<T> fromLatest ()
  {
    return new ChannelReader<>(latest);
  }
}
