
class Config
{
  static enum Mode { OFF, VALIDATE_OWNERSHIP, DETECT_DEADLOCKS };
  private static final Mode mode = Mode.valueOf(
    System.getProperty("mode", Mode.DETECT_DEADLOCKS.name()));
  public static final boolean VALIDATE_OWNERSHIP =
    mode == Mode.VALIDATE_OWNERSHIP || mode == Mode.DETECT_DEADLOCKS;
  public static final boolean DETECT_DEADLOCKS =
    mode == Mode.DETECT_DEADLOCKS;
  public static final int GC_OWNED_LIST_THRESHOLD = Integer.MAX_VALUE;
  public static final boolean DEFAULT_EXECUTOR = true;
}
