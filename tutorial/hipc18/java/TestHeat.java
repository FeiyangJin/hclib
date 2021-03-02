
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class TestHeat extends Test
{
  static {
    theClass = TestHeat.class;
  }

  int N = 50; // tasks
  int C = 20000;  // chunk size
  int ITERS = 10000;
  private final double ALPHA = 0.1;
  private final Cell [] cells = new Cell[N];
  private CheckedCompletableFuture<Void> go;

  class Cell implements IPromiseCollection
  {
    final double [] heat;
    final Channel<Double> nextLeft = new Channel<>();
    final Channel<Double> nextRight = new Channel<>();

    Cell (double [] h)
    {
      heat = h;
    }

    @Override
    public List<CheckedCompletableFuture> getPromises ()
    {
      return IPromiseCollection.merge(nextLeft, nextRight);
    }

    void update (double left, double right)
    {
      heat[0] = heat[0] + ALPHA*(left + heat[1] - 2*heat[0]);
      nextLeft.put(heat[0]);
      heat[C-1] = heat[C-1] + ALPHA*(heat[C-2] + right - 2*heat[C-1]);
      nextRight.put(heat[C-1]);
      for (int i = 1; i < C-1; i++)
        heat[i] = heat[i] + ALPHA*(heat[i-1] + heat[i+1] - 2*heat[i]);
    }

    void iterate (Iterator<Double> left, Iterator<Double> right)
    {
      go.join();
      nextLeft.put(heat[0]);
      nextRight.put(heat[C-1]);

      int iters = ITERS;
      while (iters-- > 0)
        update(left.next(), right.next());
      nextLeft.terminate();
      nextRight.terminate();
    }
  }

  @Override
  protected void entryPoint (String [] args)
  {
    if (args.length < 3) {
      System.out.println("Using default arguments.");
    } else {
      N = Integer.valueOf(args[0]);
      C = Integer.valueOf(args[1]);
      ITERS = Integer.valueOf(args[2]);
    }

    for (int i = 0; i < N; i++) {
      double [] h = new double[C];
      for (int j = 0; j < C; j++) {
        h[j] = i*N+j;
        h[j] *= h[j];
      }
      cells[i] = new Cell(h);
    }
    go = new CheckedCompletableFuture<>();

    ArrayList<CheckedCompletableFuture> tasks = new ArrayList<>();
    for (int i = 0; i < N; i++) {
      Cell self = cells[i];
      Iterator<Double> left =
        cells[i == 0 ? N-1 : i-1].nextRight.fromLatest().iterator();
      Iterator<Double> right =
        cells[i == N-1 ? 0 : i+1].nextLeft.fromLatest().iterator();
      tasks.add(AnnotatedTask.async(self).submit(
                  () -> self.iterate(left, right)));
    }

    go.complete(null);
    for (CheckedCompletableFuture cf : tasks)
      cf.join();
  }
}
