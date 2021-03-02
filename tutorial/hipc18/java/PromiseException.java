
class PromiseException extends RuntimeException
{
  PromiseException (String message, Throwable cause)
  {
    super(message, cause);
  }

  PromiseException (String message)
  {
    super(message);
  }
}
