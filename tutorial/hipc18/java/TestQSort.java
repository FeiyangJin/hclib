
import java.util.Random;

public class TestQSort extends Test
{
  static {
    theClass = TestQSort.class;
  }

  int N = 1000000;
  int threshold = 20;
  int R = 5;
  final Random rand = new Random(1234);
  int [] data;

  int partition (int left, int right)
  {
    int i = left;
    int j = right;
    int tmp;
    int pivot = data[(left + right) / 2];

    while (i <= j) {
      while (data[i] < pivot) i++;
      while (data[j] > pivot) j--;
      if (i <= j) {
        tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
        i++;
        j--;
      }
    }

    return i;
  }

  void sort (int left, int right)
  {
    int index = partition(left, right);
    if (right - left + 1 > threshold) {
      CheckedCompletableFuture t1 = null;
      CheckedCompletableFuture t2 = null;
      if (left < index - 1)
        t1 = AnnotatedTask.async().submit(() -> sort(left, index-1));
      if (index < right)
        t2 = AnnotatedTask.async().submit(() -> sort(index, right));
      if (t1 != null)
        t1.join();
      if (t2 != null)
        t2.join();
    } else {
      if (left < index - 1)
        sort(left, index-1);
      if (index < right)
        sort(index, right);
    }
  }

  @Override
  protected void entryPoint (String [] args)
  {
    if (args.length < 3) {
      System.out.println("Using default arguments.");
    } else {
      N = Integer.valueOf(args[0]);
      threshold = Integer.valueOf(args[1]);
      R = Integer.valueOf(args[2]);
    }

    data = new int[N];

    for (int r = 0; r < R; r++) {
      for (int i = 0; i < N; i++)
        data[i] = rand.nextInt();

      sort(0, N-1);
    }
  }
}

