
import java.util.Arrays;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.DoubleAdder;

abstract public class Test
{
  static protected Class<?> theClass = null;

  private static String [] args;

  private static enum Metric { NONE, STATS, TIME, MEMORY, MEMORY_ONESHOT };
  private static final Metric metric = Metric.valueOf(
    System.getProperty("metric", Metric.NONE.name()));
  public static final boolean COLLECT_STATS = metric == Metric.STATS;
  static AtomicLong taskCount = new AtomicLong(0);
  static AtomicLong promiseCount = new AtomicLong(0);
  static AtomicLong waitCount = new AtomicLong(0);

  private static final int WARMUP_RUNS = 5;
  private static final int MEASURED_RUNS = 30;

  private static final int SAMPLE_MS = 10;

  private static final Runtime runtime = Runtime.getRuntime();

  private static void runTest ()
  {
    CompletableFuture<Void> terminated = new CompletableFuture<>();
    AnnotatedTask.async().submit(() -> {
        Test test;
        try {
          test = (Test) theClass.getConstructor().newInstance();
          test.entryPoint(args);
          terminated.complete(null);
        } catch (Throwable t) {
          terminated.completeExceptionally(t);
        }
      });
    terminated.join();
  }

  static private void gc ()
  {
    runtime.gc();
    runtime.gc();
    runtime.gc();
    try { Thread.sleep(10); } catch (Throwable t) {}
    runtime.gc();
    runtime.gc();
    runtime.gc();
  }

  abstract protected void entryPoint (String [] args);

  public static void main (String [] args)
  {
    Test.args = args;
    try {
      test();
    } finally {
      OwnershipTrackingExecutor.defaultInstance.shutdown();
    }
  }

  private static long kBInUse ()
  {
    // Have to get free memory first, for robustness.
    long free = runtime.freeMemory();
    long total = runtime.totalMemory();
    return (total-free)/1024;
  }

  private static double timingRun ()
  {
    long time = System.currentTimeMillis();
    runTest();
    double seconds = (System.currentTimeMillis() - time)/1000.0;
    gc();
    return seconds;
  }

  private static double memoryRun ()
  {
    AtomicBoolean done = new AtomicBoolean(false);
    AtomicLong totalSamples = new AtomicLong(0);
    DoubleAdder kbytes = new DoubleAdder();
    Thread memoryMonitor = new Thread(() -> {
        long samples = 0;
        while (!done.get()) {
          try {
            Thread.sleep(SAMPLE_MS);
          } catch (Exception e) {
            e.printStackTrace(System.out);
          }
          kbytes.add(kBInUse());
          samples++;
        }
        totalSamples.set(samples);
      });

    memoryMonitor.start();
    runTest();
    done.set(true);
    try {
      memoryMonitor.join();
    } catch (Exception e) {
      e.printStackTrace(System.out);
    }
    gc();

    return kbytes.sum()/totalSamples.get();
  }

  private static void test ()
  {
    switch (metric) {
    case NONE:
    {
      runTest();
      break;
    }
    case STATS:
    {
      runTest();

      System.out.println(taskCount.get() + " TASKS, "
                         + promiseCount.get() + " PROMISES, "
                         + waitCount.get() + " WAITS");
      break;
    }
    case TIME:
    {
      for (int i = 0; i < WARMUP_RUNS; i++)
        timingRun();

      double [] times = new double[MEASURED_RUNS];
      double meanTime = 0;
      for (int i = 0; i < MEASURED_RUNS; i++) {
        times[i] = timingRun();
        meanTime += times[i];
      }

      meanTime /= MEASURED_RUNS;

      System.out.println(String.valueOf(meanTime) + " SECONDS "
                         + "(data: " + Arrays.toString(times));
      break;
    }
    case MEMORY:
    {
      for (int i = 0; i < WARMUP_RUNS; i++)
        memoryRun();

      double [] kbytes = new double[MEASURED_RUNS];
      double meanKBytes = 0;
      for (int i = 0; i < MEASURED_RUNS; i++) {
        kbytes[i] = memoryRun();
        meanKBytes += kbytes[i];
      }

      meanKBytes /= MEASURED_RUNS;

      System.out.println(String.valueOf(meanKBytes) + " AVG KBYTES "
                         + "(data: " + Arrays.toString(kbytes));
      break;
    }
    case MEMORY_ONESHOT:
    {
      double kbytes = memoryRun();

      System.out.println(String.valueOf(kbytes) + " KBYTES");
      break;
    }
    }
  }
}
