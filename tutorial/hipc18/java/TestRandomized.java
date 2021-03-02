
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.Future;

public class TestRandomized extends Test
{
  static {
    theClass = TestRandomized.class;
  }

  int N = 200;
  int P = 100;
  int T = 3;
  double W = .2;
  int R = 1234567;
  final ArrayList<CheckedCompletableFuture<Void>> allPromises =
    new ArrayList<>();
  double [] data;

  static class PromiseCollection extends ArrayList<CheckedCompletableFuture>
    implements IPromiseCollection
  {
    PromiseCollection (ArrayList<CheckedCompletableFuture> list)
    {
      super(list);
    }

    PromiseCollection split ()
    {
      ArrayList<CheckedCompletableFuture> splitOff = new ArrayList<>();
      for (int i = size()/2; i < size(); i++)
        splitOff.add(get(i));
      removeRange(size()/2, size());

      return new PromiseCollection(splitOff);
    }

    @Override
    public List<CheckedCompletableFuture> getPromises ()
    {
      return this;
    }
  }

  @Override
  protected void entryPoint (String [] args)
  {
    if (args.length < 5) {
      System.out.println("Using default arguments.");
    } else {
      N = Integer.valueOf(args[0]);
      P = Integer.valueOf(args[1]);
      T = Integer.valueOf(args[2]);
      W = Double.valueOf(args[3]);
      R = Integer.valueOf(args[4]);
    }

    data = new double[N];

    for (int i = 0; i < P; i++)
      allPromises.add(new CheckedCompletableFuture<>());
    PromiseCollection promises =
      new PromiseCollection(new ArrayList<>(allPromises));

    fulfill(promises, R);
  }

  void fulfill (PromiseCollection promises, long seed) {
    Random rand = new Random(seed);

    ArrayList<CheckedCompletableFuture<Void>> tasks = new ArrayList<>();
    for (int t = 0; t < T && promises.size() > 1; t++) {
      final PromiseCollection pc = promises.split();
      final long newSeed = rand.nextLong();
      tasks.add(AnnotatedTask.async(pc).submit(
                  () -> fulfill(pc, newSeed)));
    }

    if (rand.nextDouble() < W) {
      int i = (int) (seed % P);
      if (i < 0) i += P;
      allPromises.get(i).join();
    }

    for (int i = 0; i < data.length; i++) {
      for (int j = 0; j < data.length; j++) {
        data[i] += data[j] * seed;
      }
    }

    for (CheckedCompletableFuture p : promises.getPromises()) {
      p.complete(null);
    }

    for (CheckedCompletableFuture cf : tasks)
      cf.join();
  }
}
