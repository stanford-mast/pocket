/*
 * ReflexClient: An NIO-based Reflex client library
 *
 * Author: Patrick Stuedi <stu@zurich.ibm.com>
 *
 * Copyright (C) 2016, IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

package com.ibm.reflex.client;

import java.nio.channels.SocketChannel;

import org.slf4j.Logger;

public class ReflexClientGroup {
	private static final Logger LOG = ReflexUtils.getLogger();
	
	public static int DEFAULT_QUEUE_DEPTH = 16;
	public static int DEFAULT_BLOCK_SIZE = 512;
	
	private int queueDepth;
	private int blockSize;
	private boolean nodelay;	
	
	public ReflexClientGroup(){
		this(DEFAULT_QUEUE_DEPTH, DEFAULT_BLOCK_SIZE, false);
	}	
	
	public ReflexClientGroup(int queueDepth, int blockSize, boolean nodelay){
		this.queueDepth = queueDepth;
		this.blockSize = blockSize;
		this.nodelay = nodelay;
		LOG.info("new ReflexClientGroup group, queueDepth " + this.queueDepth + ", blockSize " + blockSize + ", nodelay " + nodelay);
	}

	public int getQueueDepth() {
		return queueDepth;
	}

	public int getBlockSize() {
		return blockSize;
	}

	public boolean isNodelay() {
		return nodelay;
	}
	
	public ReflexEndpoint createEndpoint() throws Exception{
		return new ReflexEndpoint(this, SocketChannel.open());
	}
}
