import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class TestSieve extends Test
{
  static {
    theClass = TestSieve.class;
  }

  int N = 100000;
  private List<Integer> primes = new ArrayList<>();

  private Iterator<Integer> ints ()
  {
    Channel<Integer> out = new Channel<>();
    Iterator<Integer> outIt = out.fromLatest().iterator();

    AnnotatedTask.async(out).submit(() -> {
        for (int i = 2; i < N; i++)
          out.put(i);
        out.terminate();
      });

    return outIt;
  }

  private Iterator<Integer> filter (int p, Iterator<Integer> in)
  {
    Channel<Integer> out = new Channel<>();
    Iterator<Integer> outIt = out.fromLatest().iterator();

    AnnotatedTask.async(out).submit(() -> {
        while (in.hasNext()) {
          int i = in.next();
          if (i % p != 0)
            out.put(i);
        }
        out.terminate();
      });

    return outIt;
  }

  @Override
  protected void entryPoint (String [] args)
  {
    if (args.length < 1) {
      System.out.println("Using default arguments.");
    } else {
      N = Integer.valueOf(args[0]);
    }

    Iterator<Integer> it = ints();
    while (it.hasNext()) {
      int p = it.next();
      primes.add(p);
      it = filter(p, it);
    }

    //System.out.println(
    //  "There are " + primes.size() + " primes less than " + N + ".");
    //System.out.println(primes);
  }
}
