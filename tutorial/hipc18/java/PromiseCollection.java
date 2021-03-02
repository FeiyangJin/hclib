
import java.util.AbstractCollection;
import java.util.Iterator;
import java.util.NoSuchElementException;

abstract class PromiseCollection extends AbstractCollection<CheckedCompletableFuture>
{
  static class EmptyPromiseCollection extends PromiseCollection
  {
    static class TheIterator implements Iterator<CheckedCompletableFuture>
    {
      @Override
      public boolean hasNext ()
      {
        return false;
      }

      @Override
      public CheckedCompletableFuture next ()
      {
        throw new NoSuchElementException();
      }
    }

    @Override
    public Iterator<CheckedCompletableFuture> iterator ()
    {
      return new TheIterator();
    }

    @Override
    public int size ()
    {
      return 0;
    }
  }

  static PromiseCollection NONE = new EmptyPromiseCollection();
}
