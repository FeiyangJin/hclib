import java.util.ArrayList;
import java.util.Iterator;

public class TestConway extends Test
{
  static {
    theClass = TestConway.class;
  }

  int N = 300;  // each worker task has an N x N chunk
  int B = 10;    // there are B x B worker tasks
  int ITERS = 5000;

  @Override
  public void entryPoint (String [] args)
  {
    if (args.length < 3) {
      System.out.println("Using default arguments.");
    } else {
      N = Integer.valueOf(args[0]);
      B = Integer.valueOf(args[1]);
      ITERS = Integer.valueOf(args[2]);
    }

    CheckedCompletableFuture<Void> launch =
      new CheckedCompletableFuture<>();

    Worker workers [][] = new Worker[B][B];
    for (int x = 0; x < B; x++) {
      for (int y = 0; y < B; y++)
        workers[x][y] = new Worker(x, y, launch);
    }
    for (int x = 0; x < B; x++) {
      for (int y = 0; y < B; y++) {
        workers[x][y].initIterators(
          workers[(x-1+B)%B][(y-1+B)%B],
          workers[(x+B)%B][(y-1+B)%B],
          workers[(x+1+B)%B][(y-1+B)%B],
          workers[(x-1+B)%B][(y+B)%B],
          workers[(x+1+B)%B][(y+B)%B],
          workers[(x-1+B)%B][(y+1+B)%B],
          workers[(x+B)%B][(y+1+B)%B],
          workers[(x+1+B)%B][(y+1+B)%B]);
      }
    }

    launch.complete(null);
    for (int x = 0; x < B; x++) {
      for (int y = 0; y < B; y++)
        workers[x][y].done.join();
    }
  }

  static class Vec
  {
    final byte [] vec;

    Vec (byte [] v)
    {
      vec = v;
    }
  }

  class Worker
  {
    byte [][] block = new byte[N+2][N+2];
    int x, y;
    Channel<Vec> toNorth, toSouth, toWest, toEast;
    Channel<Byte> toNE, toSE, toSW, toNW;
    Iterator<Vec> fromNorth, fromSouth, fromWest, fromEast;
    Iterator<Byte> fromNE, fromSE, fromSW, fromNW;
    CheckedCompletableFuture<Void> done;

    Worker (int x, int y, CheckedCompletableFuture<Void> launch)
    {
      this.x = x;
      this.y = y;
      toNorth = new Channel<>();
      toSouth = new Channel<>();
      toWest = new Channel<>();
      toEast = new Channel<>();
      toNE = new Channel<>();
      toSE = new Channel<>();
      toSW = new Channel<>();
      toNW = new Channel<>();

      done = AnnotatedTask.async(toNorth, toSouth, toWest, toEast, toNE, toSE, toSW, toNW)
        .submit(() -> {
            launch.join();
            for (int i = 0; i < ITERS; i++) {
              distribute();
              collect();
              update();
            }
            shutdown();
          });
    }

    void initIterators (Worker nw, Worker w, Worker sw,
                        Worker n, Worker s,
                        Worker ne, Worker e, Worker se)
    {
      fromNorth = n.toSouth.fromLatest().iterator();
      fromSouth = s.toNorth.fromLatest().iterator();
      fromWest = w.toEast.fromLatest().iterator();
      fromEast = e.toWest.fromLatest().iterator();
      fromNE = ne.toSW.fromLatest().iterator();
      fromSE = se.toNW.fromLatest().iterator();
      fromSW = sw.toNE.fromLatest().iterator();
      fromNW = nw.toSE.fromLatest().iterator();
    }

    private void shutdown ()
    {
      toNorth.terminate();
      toSouth.terminate();
      toWest.terminate();
      toEast.terminate();
      toNE.terminate();
      toSE.terminate();
      toSW.terminate();
      toNW.terminate();
    }

    private void distribute ()
    {
      byte [] top = new byte[N];
      byte [] bottom = new byte[N];
      byte [] left = new byte[N];
      byte [] right = new byte[N];
      for (int i = 0; i < N; i++) {
        top[i] = block[1][i+1];
        bottom[i] = block[N-2][i+1];
        left[i] = block[i+1][1];
        right[i] = block[i+1][N-2];
      }

      toNorth.put(new Vec(top));
      toSouth.put(new Vec(bottom));
      toWest.put(new Vec(left));
      toEast.put(new Vec(right));
      toNE.put(top[N-1]);
      toSE.put(bottom[N-1]);
      toSW.put(bottom[0]);
      toNW.put(top[0]);
    }

    private void collect ()
    {
      byte [] top = fromNorth.next().vec;
      byte [] bottom = fromSouth.next().vec;
      byte [] left = fromWest.next().vec;
      byte [] right = fromEast.next().vec;
      block[0][N-1] = fromNE.next();
      block[N-1][N-1] = fromSE.next();
      block[N-1][0] = fromSW.next();
      block[0][0] = fromNW.next();

      for (int i = 0; i < N; i++) {
        block[1][i+1] = top[i];
        block[N-2][i+1] = bottom[i];
        block[i+1][1] = left[i];
        block[i+1][N-2] = right[i];
      }
    }

    private void update ()
    {
      byte [][] newblock = new byte[N+1][N+1];
      for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < N-1; j++) {
          byte nbrs = (byte)
            (block[i-1][j-1] + block[i][j-1] + block[i+1][j-1]
             + block[i-1][j] + block[i+1][j]
             + block[i-1][j+1] + block[i][j+1] + block[i+1][j+1]);
          if (block[i][j] == 1)
            newblock[i][j] = nbrs == 3 ? (byte) 1 : 0;
          else
            newblock[i][j] = (nbrs == 2 || nbrs == 3) ? (byte) 1 : 0;
        }
      }
      block = newblock;
    }
  }
}
