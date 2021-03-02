
import java.util.Iterator;

public class ChannelReader <T> implements Iterable<T>
{
  private final CheckedCompletableFuture<Channel.Payload<T>> position;

  ChannelReader (CheckedCompletableFuture<Channel.Payload<T>> position)
  {
    this.position = position;
  }

  @Override
  public Iterator<T> iterator ()
  {
    return new ChannelReaderIterator<>(position);
  }

  static private class ChannelReaderIterator <T> implements Iterator<T>
  {
    private CheckedCompletableFuture<Channel.Payload<T>> current;

    ChannelReaderIterator (CheckedCompletableFuture<Channel.Payload<T>> start)
    {
      current = start;
    }

    @Override
    public boolean hasNext ()
    {
      return (current.join() != null);
    }

    @Override
    public T next ()
    {
      Channel.Payload<T> p = current.join();
      current = p.next;
      return p.value;
    }
  }
}
