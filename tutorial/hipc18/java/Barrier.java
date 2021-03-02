
import java.util.List;
import java.util.Iterator;
import java.util.concurrent.Future;

public class Barrier
{
  private BarrierParty [] parties;

  public Barrier (int num_parties)
  {
    parties = new BarrierParty[num_parties];
    for (int i = 0; i < parties.length; i++)
      parties[i] = new BarrierParty(i);
    for (int i = 0; i < parties.length; i++)
      parties[i].setFutures();
  }

  class BarrierParty implements IPromiseCollection
  {
    private final int myParty;
    private final Channel<Void> chan = new Channel<>();
    private final Iterator [] futures = new Iterator[parties.length-1];

    private BarrierParty (int party)
    {
      this.myParty = party;
    }

    @Override
    public List<CheckedCompletableFuture> getPromises ()
    {
      return chan.getPromises();
    }

    private void setFutures ()
    {
      for (int i = 0; i < parties.length; i++) {
        if (i == myParty) continue;
        int j = i > myParty ? i-1 : i;
        futures[j] = parties[i].chan.fromLatest().iterator();
      }
    }

    public void arrive ()
    {
      chan.put(null);
      for (Iterator f : futures)
        f.next();
    }

    public void terminate ()
    {
      chan.terminate();
    }
  }

  public BarrierParty getParty (int party)
  {
    return parties[party];
  }
}
