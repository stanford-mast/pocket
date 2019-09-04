package org.apache.crail;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by atr on 11.04.18.
 */
abstract public class IOCtlResponse {
    public abstract int write(ByteBuffer buffer) throws IOException;
    public abstract void update(ByteBuffer buffer) throws IOException;
    public abstract int getSize();
    public abstract int ioctlErrorCode();

    public static class IOCtlNopResp extends IOCtlResponse {
        public static int CSIZE = 0;

        public IOCtlNopResp(){
        }

        @Override
        public int write(ByteBuffer buffer) throws IOException {
            return IOCtlNopResp.CSIZE;
        }

        @Override
        public void update(ByteBuffer buffer) throws IOException {
        }

        @Override
        public int getSize(){
            return IOCtlNopResp.CSIZE;
        }

        @Override
        public int ioctlErrorCode() {
            return 0;
        }

        public String toString(){
            return "IOCtlNopResp: Empty";
        }
    }

    public static class IOCtlVoidResp extends IOCtlResponse {
        public static int CSIZE = Integer.BYTES;
        private int ecode;

        public IOCtlVoidResp(int ecode){
            this.ecode = ecode;
        }

        public IOCtlVoidResp(){
            this.ecode = -1;
        }

        @Override
        public int write(ByteBuffer buffer) throws IOException {
            buffer.putInt(this.ecode);
            return Integer.BYTES;
        }

        @Override
        public void update(ByteBuffer buffer) throws IOException {
            this.ecode = buffer.getInt();
        }

        @Override
        public int getSize(){
            return IOCtlVoidResp.CSIZE;
        }

        @Override
        public int ioctlErrorCode() {
            return this.ecode;
        }

        @Override
        public String toString(){
            return "IOCtlVoidResp: ecode: " + ecode;
        }
    }

    public static class IOCtlDataNodeRemoveResp extends IOCtlResponse {
        public static int CSIZE = 0;

        public IOCtlDataNodeRemoveResp(){
        }

        @Override
        public int write(ByteBuffer buffer) throws IOException {
            return IOCtlDataNodeRemoveResp.CSIZE;
        }

        @Override
        public void update(ByteBuffer buffer) throws IOException {
        }

        @Override
        public int getSize(){
            return IOCtlDataNodeRemoveResp.CSIZE;
        }

        @Override
        public int ioctlErrorCode() {
            return 0;
        }

        @Override
        public String toString(){
            return "IOCtlResponse: Empty";
        }
    }

    public static class GetClassStatResp extends IOCtlResponse {
        public static int CSIZE = 16;
        private long allBlocks;
        private long freeBlocks;

        public GetClassStatResp(){
            this.allBlocks = -1;
            this.freeBlocks = -1;
        }

        public GetClassStatResp(long all, long consumed){
            this.allBlocks = all;
            this.freeBlocks = consumed;
        }

        @Override
        public int write(ByteBuffer buffer) throws IOException {
            if(GetClassStatResp.CSIZE > buffer.remaining()) {
                throw new IOException("Write ByteBuffer is too small, remaining " + buffer.remaining() + " expected, " + GetClassStatResp.CSIZE + " bytes");
            }
            // write 2 longs
            buffer.putLong(this.allBlocks);
            buffer.putLong(this.freeBlocks);
            return GetClassStatResp.CSIZE;
        }

        @Override
        public void update(ByteBuffer buffer) throws IOException {
            if(getSize() > buffer.remaining()) {
                throw new IOException("Read ByteBuffer is too small, remaining " + buffer.remaining() + " expected, " + getSize() + " bytes");
            }
            this.allBlocks = buffer.getLong();
            this.freeBlocks = buffer.getLong();
        }

        @Override
        public int ioctlErrorCode() {
            return 0;
        }

        @Override
        public int getSize(){
            return GetClassStatResp.CSIZE;
        }

        @Override
        public String toString(){
            return "GetClassStatResp: all block: " + this.allBlocks + " free: " + this.freeBlocks;
        }
    }

    public static class CountFilesResp extends IOCtlResponse {
        public static int CSIZE = Long.BYTES;
        private long fileCount;

        public CountFilesResp(){
            this.fileCount = -1;
        }

        public CountFilesResp(long count){
            this.fileCount = count;
        }

        @Override
        public int write(ByteBuffer buffer) throws IOException {
            if(CountFilesResp.CSIZE > buffer.remaining()) {
                throw new IOException("Write ByteBuffer is too small, remaining " + buffer.remaining() + " expected, " + CountFilesResp.CSIZE + " bytes");
            }
            buffer.putLong(this.fileCount);
            return CountFilesResp.CSIZE;
        }

        @Override
        public void update(ByteBuffer buffer) throws IOException {
            if(getSize() > buffer.remaining()) {
                throw new IOException("Read ByteBuffer is too small, remaining " + buffer.remaining() + " expected, " + getSize() + " bytes");
            }
            this.fileCount = buffer.getLong();
        }

        @Override
        public int ioctlErrorCode() {
            return 0;
        }

        @Override
        public int getSize(){
            return CountFilesResp.CSIZE;
        }

        @Override
        public String toString(){
            return "CountFilesResp: number of files are : " + this.fileCount;
        }
    }
}
