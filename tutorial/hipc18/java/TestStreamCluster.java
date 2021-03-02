
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class TestStreamCluster extends Test
{
  static {
    theClass = TestStreamCluster.class;
  }

  // Number of threads.
  private final int NPROC = 8;

  private final int DIM = 128;
  private final int N = 102400;

  private final int kmin = 2;
  private final int kmax = 20;

  private final int chunksize = 25600;

  // Number of repetitions of pspeedy().
  // Note: Increasing SP reduces probability of random error and
  // increases running time.
  // Note: Require SP >= 1.
  private final int SP = 1;

  // Iterate ITER * k log k times.
  // Note: Increasing ITER scales running time almost linearly and
  // increases probability of getting the correct number of centers.
  // Note: Require ITER >= 1.
  private final int ITER = 3;

  // Cache line length in bytes.
  // Note: Stride lengths are multiples of this.
  private final int CACHE_LINE = 32;

  private Point [] p = new Point[chunksize];
  private boolean [] is_center = new boolean[chunksize];
  private Barrier barrier;
  private int kcenter;

  private final Random rand = new Random();

  private class Point
  {
    double weight = 1.0;
    double [] coord = rand.doubles(DIM).toArray();
    int assign = 0;
    double cost = 0.0;
  }

  // tells whether two points of D dimensions are identical
  private boolean isIdentical (double [] i, double [] j, int D)
  {
    for (int k = 0; k < i.length; k++) {
      if (i[k] != j[k])
        return false;
    }
    return true;
  }

  /* comparator for floating point numbers */
  private int doublecomp (double a, double b)
  {
    if (a > b) return 1;
    if (a < b) return -1;
    return 0;
  }

  /* shuffle points into random order */
  private void shuffle ()
  {
    for (int i = 0; i < p.length - 1; i++) {
      int j = i + rand.nextInt(p.length - i /*exclusive*/);
      Point temp = p[i];
      p[i] = p[j];
      p[j] = temp;
    }
  }

  /* shuffle an array of integers */
  private void intshuffle (int [] intarray)
  {
    for (int i = 0; i < intarray.length; i++) {
      int j = i + rand.nextInt(intarray.length - i /*exclusive*/);
      int temp = intarray[i];
      intarray[i] = intarray[j];
      intarray[j] = temp;
    }
  }

  /* compute Euclidean distance squared between two points */
  private double dist (Point p1, Point p2, int dim)
  {
    double result = 0.0;
    for (int i = 0; i < dim; i++)
      result += (p1.coord[i] - p2.coord[i])*(p1.coord[i] - p2.coord[i]);
    return result;
  }

  private double pspeedy_totalcost;
  private CheckedCompletableFuture<Void> pspeedy_open;
  private double [] pspeedy_costs = new double[NPROC];
  private int pspeedy_i;

  private double pspeedy (double z, int pid)
  {
    barrier.getParty(pid).arrive();

    //my block
    int bsize = p.length/NPROC;
    int k1 = bsize * pid;
    int k2 = k1 + bsize;
    if (pid == NPROC-1)
      k2 = p.length;

    /* create center at first point, send it to itself */
    for (int k = k1; k < k2; k++) {
      double distance = dist(p[k], p[0], DIM);
      p[k].cost = distance * p[k].weight;
      p[k].assign = 0;
    }

    if (pid==0) {
      kcenter = 1;
      pspeedy_open = new CheckedCompletableFuture<>();
    }

    barrier.getParty(pid).arrive();

    if (pid != 0) {
      // we are not the master threads. we wait until a center is opened.
      while (true) {
        pspeedy_open.join();
        //pspeedy_mutex.lock();
        //while (!pspeedy_open)
        //    pspeedy_cond.wait(pspeedy_mutex);
        //pspeedy_mutex.unlock();

        if (pspeedy_i >= p.length)
          break;
        for (int k = k1; k < k2; k++) {
          double distance = dist(p[pspeedy_i], p[k], DIM);
          if (distance*p[k].weight < p[k].cost) {
            p[k].cost = distance * p[k].weight;
            p[k].assign = pspeedy_i;
          }
        }
        barrier.getParty(pid).arrive();
        barrier.getParty(pid).arrive(); // <-- Deadlock if omitted.
      }
    } else {
      // I am the master thread. I decide whether to open a center and
      // notify others if so.
      for (pspeedy_i = 1; pspeedy_i < p.length; pspeedy_i++) {
        boolean to_open = rand.nextDouble() < (p[pspeedy_i].cost/z);
        if (to_open) {
          kcenter++;

          //pspeedy_mutex.lock();
          pspeedy_open.complete(null);
          //pspeedy_mutex.unlock();
          //pspeedy_cond.broadcast();

          for (int k = k1; k < k2; k++) {
            double distance = dist(p[pspeedy_i], p[k], DIM);
            if (distance*p[k].weight < p[k].cost) {
              p[k].cost = distance * p[k].weight;
              p[k].assign = pspeedy_i;
            }
          }

          barrier.getParty(pid).arrive();
          pspeedy_open = new CheckedCompletableFuture<>();
          barrier.getParty(pid).arrive();
        }
      }

      //pspeedy_mutex.lock();
      pspeedy_open.complete(null);
      //pspeedy_mutex.unlock();
      //pspeedy_cond.broadcast();
    }

    barrier.getParty(pid).arrive();
    //pspeedy_open = false;
    double mytotal = 0;
    for (int k = k1; k < k2; k++) {
      mytotal += p[k].cost;
    }
    pspeedy_costs[pid] = mytotal;

    barrier.getParty(pid).arrive();

    // aggregate costs from each thread
    if (pid == 0) {
      pspeedy_totalcost = z*kcenter;
      for (int j = 0; j < NPROC; j++) {
        pspeedy_totalcost += pspeedy_costs[j];
      }
    }

    barrier.getParty(pid).arrive();

    return pspeedy_totalcost;
  }


  /* For a given point x, find the cost of the following operation:
   * -- open a facility at x if there isn't already one there,
   * -- for points y such that the assignment distance of y exceeds dist(y, x),
   *    make y a member of x,
   * -- for facilities y such that reassigning y and all its members to x
   *    would save cost, realize this closing and reassignment.
   *
   * If the cost of this operation is negative (i.e., if this entire operation
   * saves cost), perform this operation and return the amount of cost saved;
   * otherwise, do nothing.
   */

  /* kcenter will be updated to reflect the new number of centers */
  /* z is the facility cost, x is the number of this point in the array
     points */

  private boolean [] pgain_switch_membership = new boolean[chunksize];
  private int [] pgain_center_table = new int[chunksize];
  private double [] pgain_work_mem;
  private double pgain_gl_cost_of_opening_x;

  private double pgain (int x, double z, int pid)
  {
    barrier.getParty(pid).arrive();

    //my block
    int bsize = p.length/NPROC;
    int k1 = bsize * pid;
    int k2 = k1 + bsize;
    if (pid == NPROC-1)
      k2 = p.length;

    int number_of_centers_to_close = 0;

    //each thread takes a block of working_mem.
    int stride = kcenter + 2;
    //make stride a multiple of CACHE_LINE
    int cl = CACHE_LINE/8; // Note: where 8=sizeof(double)
    if (stride % cl != 0) {
      stride = cl * (stride / cl + 1);
    }
    int K = stride - 2; // K==kcenter

    //my own cost of opening x
    double cost_of_opening_x = 0;

    int gl_number_of_centers_to_close = 0;
    if (pid==0) {
      pgain_work_mem = new double[stride*(NPROC+1)];
      pgain_gl_cost_of_opening_x = 0;
    }

    barrier.getParty(pid).arrive();

    /*For each center, we have a *lower* field that indicates
      how much we will save by closing the center.
      Each thread has its own copy of the *lower* fields as an array.
      We first build a table to index the positions of the *lower* fields.
    */

    int count = 0;
    for (int i = k1; i < k2; i++) {
      if (is_center[i]) {
        pgain_center_table[i] = count++;
      }
    }
    pgain_work_mem[pid*stride] = count;

    barrier.getParty(pid).arrive();

    if (pid == 0) {
      int accum = 0;
      for (int proc = 0; proc < NPROC; proc++) {
        int tmp = (int) pgain_work_mem[proc*stride];
        pgain_work_mem[proc*stride] = accum;
        accum += tmp;
      }
    }

    barrier.getParty(pid).arrive();

    for (int i = k1; i < k2; i++) {
      if (is_center[i]) {
        pgain_center_table[i] += (int) pgain_work_mem[pid*stride];
      }
    }

    //now we finish building the table. clear the working memory.
    for (int i = k1; i < k2; i++)
      pgain_switch_membership[i] = false;
    for (int i = 0; i < stride; i++)
      pgain_work_mem[pid*stride + i] = 0;
    if (pid == 0) {
      for (int i = 0; i < stride; i++)
        pgain_work_mem[NPROC*stride + i] = 0;
    }

    barrier.getParty(pid).arrive();

    //my *lower* fields
    int lowerI = pid*stride; // offset into pgain_work_mem
    //global *lower* fields
    int gl_lowerI = NPROC*stride; // offset into pgain_work_mem

    for (int i = k1; i < k2; i++) {
      double x_cost = dist(p[i], p[x], DIM) * p[i].weight;
      double current_cost = p[i].cost;

      if (x_cost < current_cost) {

        // point i would save cost just by switching to x
        // (note that i cannot be a median,
        // or else dist(p[i], p[x]) would be 0)

        pgain_switch_membership[i] = true;
        cost_of_opening_x += x_cost - current_cost;

      } else {

        // cost of assigning i to x is at least current assignment cost of i

        // consider the savings that i's **current** median would realize
        // if we reassigned that median and all its members to x;
        // note we've already accounted for the fact that the median
        // would save z by closing; now we have to subtract from the savings
        // the extra cost of reassigning that median and its members
        int assign = p[i].assign;
        pgain_work_mem[lowerI + pgain_center_table[assign]]
          += current_cost - x_cost;
      }
    }

    barrier.getParty(pid).arrive();

    // at this time, we can calculate the cost of opening a center
    // at x; if it is negative, we'll go through with opening it

    for (int i = k1; i < k2; i++) {
      if (is_center[i]) {
        double low = z;
        //aggregate from all threads
        for (int p = 0; p < NPROC; p++)
          low += pgain_work_mem[pgain_center_table[i]+p*stride];
        pgain_work_mem[gl_lowerI + pgain_center_table[i]] = low;
        if (low > 0) {
          // i is a median, and
          // if we were to open x (which we still may not) we'd close i

          // note, we'll ignore the following quantity unless we do open x
          number_of_centers_to_close++;
          cost_of_opening_x -= low;
        }
      }
    }
    //use the rest of working memory to store the following
    pgain_work_mem[pid*stride + K] = number_of_centers_to_close;
    pgain_work_mem[pid*stride + K+1] = cost_of_opening_x;

    barrier.getParty(pid).arrive();

    if (pid==0) {
      pgain_gl_cost_of_opening_x = z;
      //aggregate
      for (int p = 0; p < NPROC; p++) {
        gl_number_of_centers_to_close +=
          (int) pgain_work_mem[p*stride + K];
        pgain_gl_cost_of_opening_x += pgain_work_mem[p*stride+K+1];
      }
    }

    barrier.getParty(pid).arrive();

    // Now, check whether opening x would save cost; if so, do it, and
    // otherwise do nothing

    if (pgain_gl_cost_of_opening_x < 0 ) {
      //  we'd save money by opening x; we'll do it
      for (int i = k1; i < k2; i++) {
        boolean close_center =
          pgain_work_mem[gl_lowerI + pgain_center_table[p[i].assign]] > 0;
        if (pgain_switch_membership[i] || close_center) {
          // Either i's median (which may be i itself) is closing,
          // or i is closer to x than to its current median
          p[i].cost = p[i].weight * dist(p[i], p[x], DIM);
          p[i].assign = x;
        }
      }
      for (int i = k1; i < k2; i++) {
        if (is_center[i]
            && pgain_work_mem[gl_lowerI + pgain_center_table[i]] > 0)
          is_center[i] = false;
      }
      if (x >= k1 && x < k2)
        is_center[x] = true;

      if (pid==0)
        kcenter += 1 - gl_number_of_centers_to_close;
    } else {
      if (pid==0)
        pgain_gl_cost_of_opening_x = 0;  // the value we'll return
      // TODO: Note: There is a race here!!
    }

    barrier.getParty(pid).arrive();

    return -pgain_gl_cost_of_opening_x;
  }

  /* facility location on the points using local search */
  /* z is the facility cost, returns the total cost and # of centers */
  /* assumes we are seeded with a reasonable solution */
  /* cost should represent this solution's cost */
  /* halt if there is < e improvement after iter calls to gain */
  /* feasible is an array of numfeasible points which may be centers */

  private double pFL (int numfeasible,
                      double z, double cost, int iter, double e, int pid)
  {
    barrier.getParty(pid).arrive();

    double change = cost;
    /* continue until we run iter iterations without improvement */
    /* stop instead if improvement is less than e */
    while (change/cost > 1.0*e) {
      change = 0.0;
      /* randomize order in which centers are considered */

      if (pid == 0)
        intshuffle(pkmedian_feasible);

      barrier.getParty(pid).arrive();

      for (int i = 0; i<iter; i++) {
        int x = i%numfeasible;
        change += pgain(pkmedian_feasible[x], z, pid);
      }
      cost -= change;

      barrier.getParty(pid).arrive();
    }
    return cost;
  }

  private int selectfeasible_fast (int kmin, int pid)
  {
    int numfeasible = Integer.min(p.length, (int)(ITER*kmin*Math.log((double)kmin)));
    pkmedian_feasible = new int[numfeasible];

    /*
       Calcuate my block.  For now this routine does not seem to
       be the bottleneck, so it is not parallelized.  When
       necessary, this can be parallelized by setting k1 and k2 to
       proper values and calling this routine from all threads (
       it is called only by thread 0 for now ).  Note that when
       parallelized, the randomization might not be the same and
       it might not be difficult to measure the parallel speed-up
       for the whole program.
    */
    //  int bsize = numfeasible;
    int k1 = 0;
    int k2 = numfeasible;

    /* not many points, all will be feasible */
    if (numfeasible == p.length) {
      for (int i = k1; i<k2; i++)
        pkmedian_feasible[i] = i;
      return numfeasible;
    }
    double [] accumweight = new double[p.length];

    accumweight[0] = p[0].weight;
    double totalweight = 0;
    for (int i = 1; i < p.length; i++)
      accumweight[i] = accumweight[i-1] + p[i].weight;
    totalweight = accumweight[p.length-1];

    for (int i = k1; i<k2; i++ ) {
      double w = rand.nextDouble() * totalweight;
      //binary search
      int l = 0;
      int r = p.length-1;
      if (accumweight[0] > w) {
        pkmedian_feasible[i] = 0;
        continue;
      }
      while (l+1 < r) {
        int k = (l+r)/2;
        if (accumweight[k] > w) {
          r = k;
        } else {
          l = k;
        }
      }
      pkmedian_feasible[i] = r;
    }

    return numfeasible;
  }

  private int pkmedian_numfeasible = 0;
  private int [] pkmedian_feasible;
  private double [] pkmedian_hizs = new double[NPROC];

  /* compute approximate kmedian on the points */
  private void pkmedian (int kmin, int kmax, int pid)
  {
    //my block
    int bsize = p.length/NPROC;
    int k1 = bsize * pid;
    int k2 = k1 + bsize;
    if (pid == NPROC-1)
      k2 = p.length;

    /* NEW: Check whether more centers than points! */
    if (p.length <= kmax) {
      /* just return all points as facilities */
      for (int kk = k1; kk<k2; kk++) {
        p[kk].assign = kk;
        p[kk].cost = 0;
      }
      return;
    }

    if (pid==0) {
      for (int i = 0; i < NPROC; i++)
        pkmedian_hizs[i] = 0;
    }

    barrier.getParty(pid).arrive();

    double myhiz = 0;
    for (int kk = k1; kk < k2; kk++)
      myhiz += dist(p[kk], p[0], DIM) * p[kk].weight;
    pkmedian_hizs[pid] = myhiz;

    barrier.getParty(pid).arrive();

    double hiz = 0.0;
    for (int i = 0; i < NPROC; i++)
      hiz += pkmedian_hizs[i];
    double loz = 0.0;
    double z = (hiz+loz)/2.0;

    if (pid == 0)
      shuffle();
    double cost = pspeedy(z, pid);

    int i=0;
    /* give speedy SP chances to get at least kmin/2 facilities */
    while ((kcenter < kmin) && (i++ < SP))
      cost = pspeedy(z, pid);

    /* if still not enough facilities, assume z is too high */
    while (kcenter < kmin) {
      if (i >= SP) {
        hiz = z;
        z = (hiz+loz)/2.0;
        i = 0;
      }
      if (pid == 0)
        shuffle();
      cost = pspeedy(z, pid);
      i++;
    }

    /* now we begin the binary search for real */
    /* must designate some points as feasible centers */
    /* this creates more consistancy between FL runs */
    /* helps to guarantee correct # of centers at the end */

    if (pid == 0) {
      pkmedian_numfeasible = selectfeasible_fast(kmin, pid);
      for (int ii = 0; ii < p.length; ii++)
        is_center[p[ii].assign] = true;
    }

    barrier.getParty(pid).arrive();

    while (true) {
      /* first get a rough estimate on the FL solution */
      cost = pFL(pkmedian_numfeasible, z, cost,
                 (int)(ITER*kmax*Math.log((double)kmax)), 0.1, pid);

      /* if number of centers seems good, try a more accurate FL */
      if (((kcenter <= (1.1)*kmax) && (kcenter >= (0.9)*kmin))
          || ((kcenter <= kmax+2) && (kcenter >= kmin-2))) {

        /* may need to run a little longer here before halting without
           improvement */
        cost = pFL(pkmedian_numfeasible, z, cost,
                   (int)(ITER*kmax*Math.log((double)kmax)), 0.001, pid);
      }

      if (kcenter > kmax) {
        /* facilities too cheap */
        /* increase facility cost and up the cost accordingly */
        loz = z;
        z = (hiz+loz)/2.0;
        cost += (z-loz)*kcenter;
      }
      if (kcenter < kmin) {
        /* facilities too expensive */
        /* decrease facility cost and reduce the cost accordingly */
        hiz = z;
        z = (hiz+loz)/2.0;
        cost += (z-hiz)*kcenter;
      }

      /* if k is good, return the result */
      /* if we're stuck, just give up and return what we have */
      if (((kcenter <= kmax) && (kcenter >= kmin)) || ((loz >= (0.999)*hiz)))
        break;

      barrier.getParty(pid).arrive();
    }
  }

  /* compute the means for the k clusters */
  private int contcenters ()
  {
    for (int i = 0; i < p.length; i++) {
      /* compute relative weight of this point to the cluster */
      if (p[i].assign != i) {
        double relweight = p[p[i].assign].weight + p[i].weight;
        relweight = p[i].weight/relweight;
        for (int j = 0; j < DIM; j++) {
          p[p[i].assign].coord[j] *= 1.0-relweight;
          p[p[i].assign].coord[j] += p[i].coord[j]*relweight;
        }
        p[p[i].assign].weight += p[i].weight;
      }
    }

    return 0;
  }

  /* copy centers from points to centers */
  private void copycenters(List<Point> centers,
                           List<Integer> centerIDs, int offset)
  {
    for (int i = 0; i < p.length; i++) {
      if (is_center[i]) {
        centers.add(p[i]);
        centerIDs.add(Integer.valueOf(i + offset));
      }
    }
  }

  private void localSearch (int kmin, int kmax)
  {
    barrier = new Barrier(NPROC);
    CheckedCompletableFuture [] tasks = new CheckedCompletableFuture[NPROC];
    CheckedCompletableFuture<Void> go = new CheckedCompletableFuture<>();
    for (int i = 0; i < NPROC; i++) {
      final int pid = i;
      tasks[pid] = AnnotatedTask.async(barrier.getParty(pid)).submit(() -> {
          go.join();
          pkmedian(kmin, kmax, pid);
          barrier.getParty(pid).terminate();
        });
    }
    go.complete(null);
    for (CheckedCompletableFuture cf : tasks)
      cf.join();
  }

  @Override
  protected void entryPoint (String [] args)
  {
    List<Point> centers = new ArrayList<>();
    List<Integer> centerIDs = new ArrayList<Integer>();
    int IDoffset = 0;

    int n = N;
    while (true) {

      int points_this_round = Integer.min(n, chunksize);
      if (points_this_round < chunksize)
        p = new Point[points_this_round];

      // Random point data.
      for (int i = 0; i < points_this_round; i++)
        p[i] = new Point();

      for (int i = 0; i < points_this_round; i++)
        is_center[i] = false;

      localSearch(kmin, kmax); // parallel

      contcenters(); /* sequential */

      n -= points_this_round;
      if (n == 0)
        break;

      copycenters(centers, centerIDs, IDoffset); /* sequential */
      IDoffset += points_this_round;
    }
  }
}

