package com.ibm.crail.storage.reflex.client;

import org.apache.crail.CrailBufferCache;
import org.apache.crail.CrailStatistics;
import org.apache.crail.conf.CrailConfiguration;
import org.apache.crail.metadata.DataNodeInfo;
import org.apache.crail.storage.StorageClient;
import org.apache.crail.storage.StorageEndpoint;
import com.ibm.crail.storage.reflex.ReFlexStorageConstants;
import org.apache.crail.utils.CrailUtils;

import org.slf4j.Logger;

import com.ibm.reflex.client.ReflexClientGroup;
import com.ibm.reflex.client.ReflexEndpoint;

import java.io.IOException;

public class ReflexStorageClient implements StorageClient {
    private ReflexClientGroup clientGroup;

    public void printConf(Logger logger) {
        ReFlexStorageConstants.printConf(logger);
    }

    public void init(CrailConfiguration crailConfiguration, String[] args) throws IOException {
        ReFlexStorageConstants.updateConstants(crailConfiguration);
        ReFlexStorageConstants.verify();
        clientGroup = new ReflexClientGroup(ReFlexStorageConstants.QUEUE_SIZE, ReFlexStorageConstants.BLOCK_SIZE, ReFlexStorageConstants.NO_DELAY);
    }

    public StorageEndpoint createEndpoint(DataNodeInfo info) throws IOException {
        try {
        	ReflexEndpoint endpoint = clientGroup.createEndpoint();
        	endpoint.connect(CrailUtils.datanodeInfo2SocketAddr(info));
            return new ReFlexStorageEndpoint(endpoint);
        } catch(Exception e){
            throw new IOException(e);
        }
    }

    @Override
    public void close() throws Exception {
    }

    public void init(CrailStatistics statistics, CrailBufferCache bufferCache, CrailConfiguration configuration,
              String[] args) throws IOException {

    }
}
