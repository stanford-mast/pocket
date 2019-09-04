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

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import org.slf4j.Logger;

public abstract class ReflexChannel {
	private static final Logger LOG = ReflexUtils.getLogger();
	
	private int blockSize;
	
	public ReflexChannel(int blockSize){
		this.blockSize = blockSize;
	}
	
	public void makeRequest(short type, long ticket, long lba, int count, ByteBuffer buffer) throws IOException {
		buffer.clear();
		buffer.putShort((short) ReflexHeader.HEADERSIZE);
		buffer.putShort(type);
		buffer.putLong(ticket);
		buffer.putLong(lba);
		buffer.putInt(count);
		buffer.clear().limit(ReflexHeader.HEADERSIZE);
	}
	
	public void fetchHeader(SocketChannel channel, ByteBuffer buffer, ReflexHeader header) throws IOException{
		buffer.clear().limit(ReflexHeader.HEADERSIZE);
		while (buffer.hasRemaining()) {
			if (channel.read(buffer) < 0){
				throw new IOException("channel is closed (!)");
			}
		}
		buffer.flip();
		header.update(buffer);
	}	
	
	public void fetchBuffer(SocketChannel channel, ReflexHeader header, ByteBuffer buffer) throws IOException{
		// do not modify the buffer here, as this buffer has the right
		// position and limit set.
		while (buffer.hasRemaining()) {
			if (channel.read(buffer) < 0) {
				throw new IOException("error when reading header from socket");
			}
			
		}
		buffer.flip();
	}	
	
	public void transmitMessage(SocketChannel channel, ByteBuffer buffer) throws IOException {
//		LOG.info("transmitting message with magic2 " + buffer.getShort(0) + ", type " + buffer.getShort(2) + ", ticket " + buffer.getLong(4) + ", threadid " + Thread.currentThread().getName());
		while(buffer.hasRemaining()){
			channel.write(buffer);
		}		
	}
	
}
