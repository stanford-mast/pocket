/*
 * Crail: A Multi-tiered Distributed Direct Access File System
 *
 * Author:
 * Jonas Pfefferle <jpf@zurich.ibm.com>
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

package com.ibm.crail.storage.reflex.client;

import org.apache.crail.CrailBuffer;
import org.apache.crail.metadata.BlockInfo;
import org.apache.crail.storage.StorageEndpoint;
import org.apache.crail.storage.StorageFuture;
import org.apache.crail.utils.CrailUtils;

import org.slf4j.Logger;

import com.ibm.reflex.client.ReflexClientGroup;
import com.ibm.reflex.client.ReflexEndpoint;
import com.ibm.reflex.client.ReflexFuture;

import java.io.IOException;
import java.nio.ByteBuffer;


public class ReFlexStorageEndpoint implements StorageEndpoint { 
	private static final Logger LOG = CrailUtils.getLogger();
	private final ReflexEndpoint endpoint;
	private final ReflexClientGroup group;
	private int sectorSize;

	public ReFlexStorageEndpoint(ReflexEndpoint endpoint) throws Exception {
		this.endpoint = endpoint;
		this.group = endpoint.getGroup();
		this.sectorSize = endpoint.getGroup().getBlockSize();
	}

	@Override
	public void close() throws IOException, InterruptedException {
		endpoint.close();
	}

	@Override
	public boolean isLocal() {
		return false;
	}

	@Override
	public StorageFuture read(CrailBuffer data, BlockInfo block, long offset)
			throws IOException, InterruptedException {
		ByteBuffer buffer = data.getByteBuffer().duplicate();
		// in case when the slice size is bigger than the block size, a single buffer will be
		// used for multiple requests. In that case, except the first, all requests will have
		// a sector aligned position but not the zero position. So this check must be updated.
		if (buffer.position() % sectorSize != 0){
			throw new IOException("Can only read from a sector aligned, " + sectorSize + " bytes, locations. Current position " + buffer.position());
		}
		if ((offset % sectorSize) != 0){
			throw new IOException("Can only read at a sector aligned, " + sectorSize + " bytes, offset, current offset " + offset);
		}
		int len = buffer.remaining();
		if ((buffer.remaining() % sectorSize) != 0){
			int count = buffer.remaining()/sectorSize;
			int cap = (count+1)*sectorSize;
			int fill = cap-buffer.remaining();
			if (buffer.position() + fill > buffer.capacity()){
				throw new IOException("Can only read multiples of sector size, remaining " + buffer.remaining() + ", capacity " + buffer.capacity() + ", limit " + buffer.limit());
			}
			buffer.limit(buffer.limit() + fill);
		}
		long lba = linearBlockAddress(block, offset, sectorSize);

		ReflexFuture future = endpoint.get(lba, buffer);
		return new ReFlexStorageFuture(future, len);
	}

	@Override
	public StorageFuture write(CrailBuffer data, BlockInfo block, long offset)
			throws IOException, InterruptedException {
		ByteBuffer buffer = data.getByteBuffer().duplicate();
		// see explanation above in the read function
		if (buffer.position() % sectorSize != 0){
			throw new IOException("Can only write to a sector aligned, " + sectorSize + " bytes, locations. Current position " + buffer.position());
		}
		if ((offset % sectorSize) != 0){
			throw new IOException("Can only write to a sector aligned, " + sectorSize + " bytes, offset, current offset " + offset);
		}
		int len = buffer.remaining();
		if ((buffer.remaining() % sectorSize) != 0){
			int count = buffer.remaining()/sectorSize;
			int cap = (count+1)*sectorSize;
			int fill = cap-buffer.remaining();
			if (buffer.position() + fill > buffer.capacity()){
				throw new IOException("Can only write multiples of sector size, remaining " + buffer.remaining() + ", capacity " + buffer.capacity() + ", limit " + buffer.limit());
			}
			buffer.limit(buffer.limit() + fill);
		}
		long lba = linearBlockAddress(block, offset, sectorSize);

		ReflexFuture future = endpoint.put(lba, buffer);
		return new ReFlexStorageFuture(future, len);
	}

	private long linearBlockAddress(BlockInfo remoteMr, long remoteOffset, int sectorSize) {
		return (remoteMr.getAddr() + remoteOffset) / (long)sectorSize;
	}	
	
	private long namespaceSectorOffset(int sectorSize, long fileOffset) {
		return fileOffset % (long)sectorSize;
	}

	private long alignLength(int sectorSize, long remoteOffset, long len) {
		long alignedSize = len + namespaceSectorOffset(sectorSize, remoteOffset);
		if (namespaceSectorOffset(sectorSize, alignedSize) != 0) {
			alignedSize += (long)sectorSize - namespaceSectorOffset(sectorSize, alignedSize);
		}
		return alignedSize;
	}

	private long alignOffset(int sectorSize, long fileOffset) {
		return fileOffset - namespaceSectorOffset(sectorSize, fileOffset);
	}
}
