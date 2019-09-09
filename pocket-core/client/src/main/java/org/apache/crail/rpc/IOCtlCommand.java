package org.apache.crail.rpc;

import org.apache.crail.WeightMask;
import org.apache.crail.metadata.FileName;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

/**
 * Created by atr on 04.04.18.
 */
public abstract class IOCtlCommand {
    public static final byte NOP       = 1;
    public static final byte DN_REMOVE = 2;
    public static final byte NN_GET_CLASS_STAT = 3;
    public static final byte NN_SET_WMASK = 4;
    public static final byte COUNT_FILES = 5;

    public abstract int write(ByteBuffer buffer) throws IOException;
    public abstract void update(ByteBuffer buffer) throws IOException;
    public abstract int getSize();

    public static class RemoveDataNode extends IOCtlCommand {
        // 4 byte IP + 4 byte port (java short are signed hence, we need 4 byte numbers
        public static int CSIZE = 8;
        private InetAddress address;
        private int port;

        RemoveDataNode(){
            this.address = null;
            this.port = -1;
        }

        public RemoveDataNode(InetAddress address, int port){
            this.address = address;
            this.port = port;
        }

        public InetAddress getIPAddress(){
            return this.address;
        }

        public int port(){
            return this.port;
        }

        public int write(ByteBuffer buffer) throws IOException {
            byte[] x = this.address.getAddress();
            if(RemoveDataNode.CSIZE > buffer.remaining()) {
                throw new IOException("Write ByteBuffer is too small, remaining " + buffer.remaining() + " expected, " + RemoveDataNode.CSIZE + " bytes");
            }
            buffer.put(x);
            buffer.putInt(this.port);
            return RemoveDataNode.CSIZE;
        }

        public void update(ByteBuffer buffer) throws IOException {
            byte[] barr = new byte[4]; // 4 bytes for the IP address
            if(getSize() > buffer.remaining()) {
                throw new IOException("Read ByteBuffer is too small, remaining " + buffer.remaining() + " expected, " + getSize() + " bytes");
            }
            buffer.get(barr);
            try {
                this.address = InetAddress.getByAddress(barr);
            } catch (UnknownHostException e) {
                e.printStackTrace();
            }
            this.port = buffer.getInt();
        }

        public int getSize(){
            return RemoveDataNode.CSIZE;
        }

        public String toString(){
            return "removeDN: " + (this.address == null ? " N/A" : (this.address.toString() + "/port: " + this.port));
        }
    }

    public static class GetClassStatCommand extends IOCtlCommand {
        public static int CSIZE = 4;
        private int storageClass;

        public GetClassStatCommand (){
            this.storageClass = -1;
        }

        public GetClassStatCommand (int storageClass){
            this.storageClass = storageClass;
        }

        public int write(ByteBuffer buffer) throws IOException {
            if(GetClassStatCommand.CSIZE > buffer.remaining()) {
                throw new IOException("Write ByteBuffer is too small, remaining " + buffer.remaining() + " expected, " + GetClassStatCommand.CSIZE + " bytes");
            }
            buffer.putInt(this.storageClass);
            return GetClassStatCommand.CSIZE;
        }

        public void update(ByteBuffer buffer) throws IOException {
            if(GetClassStatCommand.CSIZE > buffer.remaining()) {
                throw new IOException("Write ByteBuffer is too small, remaining " + buffer.remaining() + " expected, " + GetClassStatCommand.CSIZE + " bytes");
            }
            this.storageClass = buffer.getInt();
        }

        public int getStorageClass(){
            return this.storageClass;
        }

        public int getSize(){ return GetClassStatCommand.CSIZE;}

        public String toString(){ return "GetClassStatCommand class: " + this.storageClass;}
    }

    public static class AttachWeigthMaskCommand extends IOCtlCommand {
        // this need the hash of the directory name
        private FileName dirLocation;
        // and a list of int,<ip,mask>
        private WeightMask mask;

        public AttachWeigthMaskCommand(){
            this.dirLocation = new FileName();
            this.mask = new WeightMask();
        }

        public AttachWeigthMaskCommand(FileName dirLocation, WeightMask mask){
            this.dirLocation = dirLocation;
            this.mask = mask;
        }

        public int write(ByteBuffer buffer) throws IOException{
            int s1 = this.dirLocation.write(buffer);
            int s2 = this.mask.write(buffer);
            return s1 + s2;
        }

        public void update(ByteBuffer buffer) throws IOException {
            this.dirLocation.update(buffer);
            this.mask.update(buffer);
        }

        public int getSize(){
            return FileName.CSIZE + this.mask.getSize();
        }

        public FileName getDirLocation(){
            return this.dirLocation;
        }

        public WeightMask getWeightMask(){
            return this.mask;
        }

        public boolean isEmpty(){
            return this.mask.isEmpty();
        }

        public String toString(){ return "AttachWeigthMaskCommand";}
    }

    public static class CountFilesCommand extends IOCtlCommand {
        // this need the hash of the directory name
        private FileName dirLocation;

        public CountFilesCommand(){
            this.dirLocation = new FileName();
        }

        public CountFilesCommand(FileName dirLocation){
            this.dirLocation = dirLocation;
        }

        public int write(ByteBuffer buffer) throws IOException{
            return this.dirLocation.write(buffer);
        }

        public void update(ByteBuffer buffer) throws IOException {
            this.dirLocation.update(buffer);
        }

        public int getSize(){
            return FileName.CSIZE;
        }

        public FileName getDirLocation(){
            return this.dirLocation;
        }

        public String toString(){ return "CountFiles";}
    }

    public static class NoOpCommand extends IOCtlCommand {

        NoOpCommand(){}

        public int write(ByteBuffer buffer){return  0;}

        public void update(ByteBuffer buffer){}

        public int getSize(){ return 0;}

        public String toString(){ return "NoOpCommand";}
    }
}
