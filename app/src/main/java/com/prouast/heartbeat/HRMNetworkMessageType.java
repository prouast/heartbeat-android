package com.prouast.heartbeat;

import android.util.Log;

/**
 * Provides constants for the different types of network messages
 */
public enum HRMNetworkMessageType {

    // Client signals that he is still listening to server
    HEARTBEAT,

    // Client sends the physio signal
    HEARTRATE,

    // Client and server exchange timestamps to determine time offset
    TIME_SYNC,

    // Server tells client to stop
    STOP;

    private static final String TAG = "Heartbeat::HRMNtwMsgTpe";
    private static final HRMNetworkMessageType[] values = HRMNetworkMessageType.values();


    /**
     * Parse a HRMNetworkMessageType from an int
     * @param i int
     * @return type
     */
    public static HRMNetworkMessageType fromInt(int i) {
        try {
            return values[i];
        } catch (ArrayIndexOutOfBoundsException e) {
            Log.e(TAG, "Exception when trying to access ordinal value " + i + ": " + e);
            e.printStackTrace();
        }
        return HEARTBEAT;
    }
}
