package com.prouast.heartbeat;

import android.util.Log;

/**
 * Provides constants for the different types of network messages
 */
public enum RPPGNetworkMessageType {

    // Client signals that he is still listening to server
    HEARTBEAT,

    // Client sends the physio signal
    HEARTRATE,

    // Client and server exchange timestamps to determine time offset
    TIME_SYNC,

    // Server tells client to stop
    STOP,

    INVALID;

    private static final String TAG = "Heartbeat::HRMNtwMsgTpe";
    private static final RPPGNetworkMessageType[] values = RPPGNetworkMessageType.values();


    /**
     * Parse a RPPGNetworkMessageType from an int
     * @param i int
     * @return type
     */
    public static RPPGNetworkMessageType fromInt(int i) {
        try {
            return values[i];
        } catch (ArrayIndexOutOfBoundsException e) {
            Log.e(TAG, "Exception when trying to access ordinal value " + i + ": " + e);
            e.printStackTrace();
        }
        return INVALID;
    }
}
