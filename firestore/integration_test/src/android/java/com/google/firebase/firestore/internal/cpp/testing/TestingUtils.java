package com.google.firebase.firestore.internal.cpp.testing;

import android.os.Handler;
import android.os.Looper;

public final class TestingUtils {

  private TestingUtils() {
  }

  public static long getMainThreadId() {
    return Looper.getMainLooper().getThread().getId();
  }

  public static long getCurrentThreadId() {
    return Thread.currentThread().getId();
  }

  public static long getAnotherThreadId() throws InterruptedException {
    CaptureThreadIdRunnable runnable = new CaptureThreadIdRunnable();
    Thread thread = new Thread(runnable);
    thread.start();
    return runnable.waitForCapturedThreadId();
  }

  public static void runOnMainThread(long data) throws InterruptedException {
    NativeRunnable runnable = new NativeRunnable(data);
    Handler handler = new Handler(Looper.getMainLooper());
    boolean postSucceeded = handler.post(runnable);
    if (!postSucceeded) {
      throw new RuntimeException("Handler.post() failed");
    }
  }

  private static final class CaptureThreadIdRunnable implements Runnable {

    private boolean threadIdCatpured = false;
    private long capturedThreadId;

    @Override
    public synchronized void run() {
      capturedThreadId = getCurrentThreadId();
      threadIdCatpured = true;
      notifyAll();
    }

    synchronized long waitForCapturedThreadId() throws InterruptedException {
      while (!threadIdCatpured) {
        wait();
      }
      return capturedThreadId;
    }
    
  }

  private static final class NativeRunnable implements Runnable {

    private final long data;

    NativeRunnable(long data) {
      this.data = data;
    }

    @Override
    public void run() {
      nativeRun(data);
    }

    private static native void nativeRun(long data);

  }

}

