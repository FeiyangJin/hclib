
import java.util.ArrayList;
import java.util.Random;
import java.util.concurrent.Future;

public class TestStrassen extends Test
{
  static {
    theClass = TestStrassen.class;
  }

  final int MAX_DEPTH = 5;
  int logSize = 8;
  int size = 1 << logSize;
  int nValues = size*size / 8;

  @Override
  protected void entryPoint (String [] args)
  {
    if (args.length < 2) {
      System.out.println("Using default arguments.");
    } else {
      logSize = Integer.valueOf(args[0]);
      size = 1 << logSize;
      nValues = Integer.valueOf(args[1]);
    }

    final Random rand = new Random(1234);

    SparseMatrix m = new QuadtreeSparseMatrix(logSize);
    for (int k = 0; k < nValues; k++) {
      int i = rand.nextInt(size);
      int j = rand.nextInt(size);
      m.setValue(i, j, 1.0);
    }

    int inputSize = m.countNonZero();
    //System.out.println("Input has " + inputSize + " nonzero entries.");

    m = times(m, m, 0);

    int outputSize = m.countNonZero();
    //System.out.println("Output has " + outputSize + " nonzero entries.");
  }

  static interface SparseMatrix
  {
    public void setValue (int i, int j, double v);
    public double getValue (int i, int j);
    public void setQuadrant (int q, SparseMatrix m);
    public SparseMatrix getQuadrant (int q);
    public boolean isEmpty ();
    public int getLogSize ();
    public int countNonZero ();

    default public void touch ()
    {
    }
  }

  static SparseMatrix lift (Future<SparseMatrix> f)
  {
    return new FutureSparseMatrix(f);
  }

  private SparseMatrix plus_ (SparseMatrix a, SparseMatrix b, int d)
  {
    if (a.isEmpty())
      return b;
    if (b.isEmpty())
      return a;
    if (a.getLogSize() != b.getLogSize())
      throw new RuntimeException("Matrix size mismatch");

    if (a.getLogSize() == 0) {
      SparseMatrix c = new QuadtreeSparseMatrix(0);
      c.setValue(0, 0, a.getValue(0, 0) + b.getValue(0, 0));
      return c;
    }

    SparseMatrix c = new QuadtreeSparseMatrix(a.getLogSize());
    for (int q = 0; q < 4; q++)
      c.setQuadrant(q, plus(a.getQuadrant(q), b.getQuadrant(q), d));
    return c;
  }

  SparseMatrix plus (SparseMatrix a, SparseMatrix b, int d)
  {
    if (d == MAX_DEPTH)
      return plus_(a, b, d);
    return lift(AnnotatedTask.async().submit(() -> plus_(a, b, d+1)));
  }

  static SparseMatrix negative (SparseMatrix a)
  {
    if (a.isEmpty())
      return a;

    return new NegatedSparseMatrix(a);
  }

  private SparseMatrix times_ (SparseMatrix a, SparseMatrix b, int d)
  {
    if (a.isEmpty())
      return a;
    if (b.isEmpty())
      return b;
    if (a.getLogSize() != b.getLogSize())
      throw new RuntimeException("Matrix size mismatch");

    if (a.getLogSize() == 0) {
      SparseMatrix c = new QuadtreeSparseMatrix(0);
      c.setValue(0, 0, a.getValue(0, 0) * b.getValue(0, 0));
      return c;
    }

    SparseMatrix s1 = plus(a.getQuadrant(0), a.getQuadrant(3), d);
    SparseMatrix s2 = plus(b.getQuadrant(0), b.getQuadrant(3), d);
    SparseMatrix m1 = times(s1, s2, d);

    SparseMatrix s3 = plus(a.getQuadrant(2), a.getQuadrant(3), d);
    SparseMatrix m2 = times(s3, b.getQuadrant(0), d);

    SparseMatrix s4 = plus(b.getQuadrant(1), negative(b.getQuadrant(3)), d);
    SparseMatrix m3 = times(a.getQuadrant(0), s4, d);

    SparseMatrix s5 = plus(b.getQuadrant(2), negative(b.getQuadrant(0)), d);
    SparseMatrix m4 = times(a.getQuadrant(3), s5, d);
    SparseMatrix c = new QuadtreeSparseMatrix(a.getLogSize());
    c.setQuadrant(2, plus(m2, m4, d));

    SparseMatrix s6 = plus(a.getQuadrant(0), a.getQuadrant(1), d);
    SparseMatrix m5 = times(s6, b.getQuadrant(3), d);
    c.setQuadrant(1, plus(m3, m5, d));

    SparseMatrix s7 = plus(a.getQuadrant(2), negative(a.getQuadrant(0)), d);
    SparseMatrix s8 = plus(b.getQuadrant(0), b.getQuadrant(1), d);
    SparseMatrix m6 = times(s7, s8, d);

    SparseMatrix s9 = plus(a.getQuadrant(1), negative(a.getQuadrant(3)), d);
    SparseMatrix s10 = plus(b.getQuadrant(2), b.getQuadrant(3), d);
    SparseMatrix m7 = times(s9, s10, d);

    SparseMatrix d3 = plus(m1, negative(m2), d);
    SparseMatrix d1 = plus(m1, m4, d);
    SparseMatrix d4 = plus(m3, m6, d);
    c.setQuadrant(3, plus(d3, d4, d));
    SparseMatrix d2 = plus(negative(m5), m7, d);
    c.setQuadrant(0, plus(d1, d2, d));

    return c;
  }

  SparseMatrix times (SparseMatrix a, SparseMatrix b, int d)
  {
    if (d == MAX_DEPTH)
      return times_(a, b, d);
    return lift(AnnotatedTask.async().submit(() -> times_(a, b, d+1)));
  }

  static abstract class SparseMatrixView implements SparseMatrix
  {
    public void setValue (int i, int j, double v)
    {
      throw new RuntimeException("Matrix view is immutable");
    }

    public void setQuadrant (int q, SparseMatrix m)
    {
      throw new RuntimeException("Matrix view is immutable");
    }
  }

  static class EmptySparseMatrix extends SparseMatrixView
  {
    private EmptySparseMatrix ()
    {
    }

    public double getValue (int i, int j)
    {
      return 0.0;
    }
    public SparseMatrix getQuadrant (int q)
    {
      return this;
    }

    public boolean isEmpty ()
    {
      return true;
    }

    public int getLogSize ()
    {
      throw new RuntimeException("Empty matrix has ill-defined size");
    }

    public int countNonZero ()
    {
      return 0;
    }
  }
  public static final EmptySparseMatrix EMPTY = new EmptySparseMatrix();

  // Quadtree-based sparse matrix implementation.
  class QuadtreeSparseMatrix implements SparseMatrix
  {
    private final int k;
    private final int n;
    private SparseMatrix [] mm = null;
    private double value;

    // logsize must be >= 0
    public QuadtreeSparseMatrix (int logSize)
    {
      k = logSize;
      n = 1 << k;

      if (k > 0) {
        mm = new SparseMatrix[4];
        for (int q = 0; q < 4; q++)
          mm[q] = EMPTY;
      }
    }

    public void setValue (int i, int j, double v)
    {
      if (k == 0) {
        if (i != 0 || j != 0)
          throw new RuntimeException("Row/column index error");
        value = v;
        return;
      }

      // Pick the quadrant.
      int mi = 0;
      if (i >= n/2) mi += 2;
      if (j >= n/2) mi++;

      // Initialize if not already.
      if (mm[mi].isEmpty())
        mm[mi] = new QuadtreeSparseMatrix(k-1);

      // Adjust the indices.
      mm[mi].setValue(i%(n/2), j%(n/2), v);
    }

    public double getValue (int i, int j)
    {
      if (k == 0) {
        if (i != 0 || j != 0)
          throw new RuntimeException("Row/column index error");
        return value;
      }

      // Pick the quadrant.
      int mi = 0;
      if (i >= n/2) mi += 2;
      if (j >= n/2) mi++;

      // Zero if not initialized.
      if (mm[mi].isEmpty())
        return 0.0;

      // Adjust the indices.
      return mm[mi].getValue(i%(n/2), j%(n/2));
    }

    public void setQuadrant (int q, SparseMatrix m)
    {
      if (mm == null)
        throw new RuntimeException("Matrix is 1x1");
      mm[q] = m;
    }

    public SparseMatrix getQuadrant (int q)
    {
      if (mm == null)
        throw new RuntimeException("Matrix is 1x1");
      return mm[q];
    }

    public boolean isEmpty ()
    {
      return false;
    }

    public int getLogSize ()
    {
      return k;
    }

    public int countNonZero ()
    {
      if (k == 0) {
        if (value == 0.0)
          return 0;
        return 1;
      }

      ArrayList<Future<Integer>> nzFutures = new ArrayList<>();
      for (int q = 0; q < 4; q++) {
        final int qq = q;
        nzFutures.add(AnnotatedTask.async().submit(() -> mm[qq].countNonZero()));
      }
      int nz = 0;
      try {
        for (Future<Integer> f : nzFutures)
          nz += f.get();
      } catch (Throwable t) {
        throw new RuntimeException(t);
      }

      return nz;
    }
  }

  static class NegatedSparseMatrix extends SparseMatrixView
  {
    private final SparseMatrix mat;

    public NegatedSparseMatrix (SparseMatrix a)
    {
      mat = a;
    }

    public double getValue (int i, int j)
    {
      return -mat.getValue(i,j);
    }

    public SparseMatrix getQuadrant (int q)
    {
      return negative(mat.getQuadrant(q));
    }

    public boolean isEmpty ()
    {
      return mat.isEmpty();
    }

    public int getLogSize ()
    {
      return mat.getLogSize();
    }

    public int countNonZero ()
    {
      return mat.countNonZero();
    }

    public void touch ()
    {
      mat.touch();
    }
  }

  static class FutureSparseMatrix extends SparseMatrixView
  {
    private final Future<SparseMatrix> f;

    public FutureSparseMatrix (Future<SparseMatrix> future)
    {
      f = future;
    }

    private SparseMatrix get ()
    {
      try {
        return f.get();
      } catch (Throwable t) {
        throw new RuntimeException(t);
      }
    }

    public double getValue (int i, int j)
    {
      return get().getValue(i,j);
    }

    public SparseMatrix getQuadrant (int q)
    {
      return get().getQuadrant(q);
    }

    public boolean isEmpty ()
    {
      return get().isEmpty();
    }

    public int getLogSize ()
    {
      return get().getLogSize();
    }

    public int countNonZero ()
    {
      return get().countNonZero();
    }

    public void touch ()
    {
      get();
    }
  }
}

