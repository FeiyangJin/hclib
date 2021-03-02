
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

interface IPromiseCollection
{
  public List<CheckedCompletableFuture> getPromises ();

  public static List<CheckedCompletableFuture> merge (IPromiseCollection... collections)
  {
    ArrayList<CheckedCompletableFuture> all = new ArrayList<>();
    if (!Config.VALIDATE_OWNERSHIP)
      return all;
    for (IPromiseCollection coll : collections)
      all.addAll(coll.getPromises());
    return all;
  }

  static class EmptyPromiseCollection implements IPromiseCollection
  {
    public List<CheckedCompletableFuture> getPromises ()
    {
      return new ArrayList<>();
    }
  }

  public static IPromiseCollection NONE = new EmptyPromiseCollection();
}
