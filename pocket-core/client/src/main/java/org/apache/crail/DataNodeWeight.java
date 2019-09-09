package org.apache.crail;

import java.nio.ByteBuffer;

/**
 * Created by atr on 24.04.18.
 */
public class DataNodeWeight {
    public long datanodeHash;
    public float weight;
    public static int CSIZE = Long.BYTES + Float.BYTES;

    public int write(ByteBuffer buffer){
        buffer.putLong(datanodeHash);
        buffer.putFloat(weight);
        return CSIZE;
    }

    public void update(ByteBuffer buffer){
        this.datanodeHash = buffer.getLong();
        this.weight = buffer.getFloat();
    }

    @Override
    public String toString() {
        return ("\t | hash: " + datanodeHash + " weight " + weight + " | \n");
    }
}
