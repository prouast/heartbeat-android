package com.prouast.heartbeat;

import java.util.ArrayList;

/**
 * A queue that holds HRMResults that have not yet been sent to the server
 */
public class RPPGResultQueue {

    // Singleton instance
    private static RPPGResultQueue instance;

    // The queue
    private ArrayList<RPPGResult> queue;

    /**
     * Singleton pattern
     * @return instance
     */
    public static RPPGResultQueue getInstance() {
        if (instance == null) {
            instance = new RPPGResultQueue();
        }
        return instance;
    }

    /**
     * Constructor instantiates
     */
    private RPPGResultQueue() {
        queue = new ArrayList<>();
    }

    /**
     *
     * @return
     */
    public synchronized RPPGResult pop() {
        while (isEmpty()) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        RPPGResult result = queue.get(0);
        queue.remove(result);

        notify();
        return result;
    }

    public synchronized void push(RPPGResult result) {
        queue.add(result);
        notify();
    }

    public boolean isEmpty() {
        return queue.isEmpty();
    }
}