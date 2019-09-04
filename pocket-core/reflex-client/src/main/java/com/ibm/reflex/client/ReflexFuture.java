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

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicBoolean;

public class ReflexFuture implements Future<ByteBuffer> {
	private ReflexEndpoint endpoint;
	private ByteBuffer buffer;
	private long ticket;
	private AtomicBoolean done;
	
	public ReflexFuture(ReflexEndpoint endpoint, long ticket, ByteBuffer buffer) {
		this.endpoint = endpoint;
		this.buffer = buffer;
		this.ticket = ticket;
		this.done = new AtomicBoolean(false);
	}

	@Override
	public boolean cancel(boolean mayInterruptIfRunning) {
		return false;
	}

	@Override
	public boolean isCancelled() {
		return false;
	}

	@Override
	public boolean isDone() {
		try {
			if (!done.get()){
				endpoint.pollResponse(done);
			}
		} catch(Exception e){
		}
		return done.get();
	}

	@Override
	public ByteBuffer get() throws InterruptedException, ExecutionException {
		try {
			while (!done.get()){
				endpoint.pollResponse(done);
			}
		} catch(Exception e){
			throw new ExecutionException(e);
		}
		return buffer;
	}

	@Override
	public ByteBuffer get(long timeout, TimeUnit unit) throws InterruptedException,
			ExecutionException, TimeoutException {
		try {
			while (!done.get()){
				endpoint.pollResponse(done);
			}
		} catch(Exception e){
			throw new ExecutionException(e);
		}
		return buffer;
	}
	
	public ByteBuffer getBuffer() {
		return buffer;
	}

	public long getTicket() {
		return ticket;
	}

	void signal() {
		this.done.set(true);
	}
}
