package org.apache.crail;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;

/**
 * Created by atr on 24.04.18.
 */
public class WeightMask {
    private ArrayList<DataNodeWeight> mask;

    public WeightMask(){
        this.mask = new ArrayList<>();
    }

    public void addMask(DataNodeWeight w){
        this.mask.add(w);
    }

    public int getSize(){
        return Integer.BYTES + (this.mask.size() * DataNodeWeight.CSIZE);
    }

    public int size(){
        return this.mask.size();
    }

    public int write(ByteBuffer buffer) throws IOException {
        int items = this.mask.size();
        int size = getSize();
        if(buffer.remaining() < size){
            throw new IOException("Size too small");
        }
        buffer.putInt(items);
        for(int i = 0; i < items; i++) {
            this.mask.get(i).write(buffer);
        }
        return size;
    }

    public void update(ByteBuffer buffer) throws IOException {
        int items = buffer.getInt();
        for(int i = 0; i < items; i++) {
            DataNodeWeight w = new DataNodeWeight();
            w.update(buffer);
            this.mask.add(w);
        }
    }

    public boolean isEmpty(){
        return this.mask.isEmpty();
    }

    @Override
    public String toString() {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("\nentries: " + this.mask.size()+"\n");
        for(int i = 0; i < this.mask.size(); i++)
            stringBuilder.append(this.mask.get(i).toString());
        return stringBuilder.toString();
    }

    public int getNextNode(){
        //https://stackoverflow.com/questions/6737283/weighted-randomness-in-java
        // Compute the total weight of all items together
        double totalWeight = 0.0;
        for(int i = 0; i < this.mask.size();i++) {
            totalWeight += this.mask.get(i).weight;
        }
        // Now choose a random item
        int randomIndex = -1;
        double random = Math.random() * totalWeight;
        for (int i = 0; i < this.mask.size(); ++i)
        {
            random -= this.mask.get(i).weight;
            if (random <= 0.0d)
            {
                randomIndex = i;
                break;
            }
        }
        return randomIndex;
    }

    public DataNodeWeight getEntry(int index){
        return this.mask.get(index);
    }
}
