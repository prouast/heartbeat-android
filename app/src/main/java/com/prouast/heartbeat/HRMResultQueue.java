package com.prouast.heartbeat;

import java.util.ArrayList;

/**
 * A queue that holds HRMResults that have not yet been sent to the server
 */
public class HRMResultQueue {

    // Singleton instance
    private static HRMResultQueue instance;

    // The queue
    private ArrayList<HRMResult> queue;

    /**
     * Singleton pattern
     * @return instance
     */
    public static HRMResultQueue getInstance() {
        if (instance == null) {
            instance = new HRMResultQueue();
        }
        return instance;
    }

    /**
     * Constructor instantiates
     */
    private HRMResultQueue() {
        queue = new ArrayList<>();
    }

    /**
     *
     * @return
     */
    public synchronized HRMResult pop() {
        while (isEmpty()) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        HRMResult result = queue.get(0);
        queue.remove(result);

        notify();
        return result;
    }

    public synchronized void push(HRMResult result) {
        queue.add(result);
        notify();
    }

    public boolean isEmpty() {
        return queue.isEmpty();
    }
}