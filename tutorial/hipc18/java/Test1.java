
public class Test1 extends Test
{
  static {
    theClass = Test1.class;
  }

  @Override
  protected void entryPoint (String [] args)
  {
    CheckedCompletableFuture<Integer> p = new CheckedCompletableFuture<>();
    //p.complete(7);
    System.out.println(p.join());
  }
}
