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

package com.ibm.crail.storage.reflex;

import org.apache.crail.conf.CrailConfiguration;
import org.apache.crail.conf.CrailConstants;
import org.slf4j.Logger;
import com.ibm.reflex.client.ReflexClientGroup;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class ReFlexStorageConstants {

	private final static String PREFIX = "crail.storage.reflex";
	
	public final static int VERSION = 3;

	public static final String IP_ADDR_KEY = "bindip";
	public static InetAddress IP_ADDR;

	public static final String PORT_KEY = "port";
	public static int PORT = 1234;

	public static final String ALLOCATION_SIZE_KEY = "allocationsize";
	public static long ALLOCATION_SIZE = 1073741824; /* 1GB */

	public static final String NAMESPACE_SIZE_KEY = "namespacesize";
	public static long NAMESPACE_SIZE = 0x1749a956000L;
	
	public static final String QUEUE_SIZE_KEY = "queuesize";
	public static int QUEUE_SIZE = ReflexClientGroup.DEFAULT_QUEUE_DEPTH; /* 1GB */	
	
	public static final String BLOCK_SIZE_KEY = "blocksize";
	public static int BLOCK_SIZE = ReflexClientGroup.DEFAULT_BLOCK_SIZE; /* 1GB */		
	
	public static final String NO_DELAY_KEY = "nodelay";
	public static boolean NO_DELAY = true; /* 1GB */		

	private static String fullKey(String key) {
		return PREFIX + "." + key;
	}

	private static String get(CrailConfiguration conf, String key) {
		return conf.get(fullKey(key));
	}

	public static void updateConstants(CrailConfiguration conf) throws UnknownHostException {
		String arg;
		arg = get(conf, IP_ADDR_KEY);
		if (arg != null) {
			IP_ADDR = InetAddress.getByName(arg);
		}

		arg = get(conf, PORT_KEY);
		if (arg != null) {
			PORT = Integer.parseInt(arg);
		}

		arg = get(conf, ALLOCATION_SIZE_KEY);
		if (arg != null) {
			ALLOCATION_SIZE = Long.parseLong(arg);
		}
		
		arg = get(conf, NAMESPACE_SIZE_KEY);
		if (arg != null) {
			NAMESPACE_SIZE = Long.parseLong(arg);
		}

		arg = get(conf, QUEUE_SIZE_KEY);
		if (arg != null) {
			QUEUE_SIZE = Integer.parseInt(arg);
		}	
		
		arg = get(conf, BLOCK_SIZE_KEY);
		if (arg != null) {
			BLOCK_SIZE = Integer.parseInt(arg);
		}	
		
		arg = get(conf, NO_DELAY_KEY);
		if (arg != null) {
			NO_DELAY = Boolean.parseBoolean(arg);
		}			
	}

	public static void verify() throws IOException {
		if (ALLOCATION_SIZE % CrailConstants.BLOCK_SIZE != 0){
			throw new IOException("allocationsize must be multiple of crail.blocksize");
		}
	}

	public static void printConf(Logger logger) {
		if (IP_ADDR != null) {
			logger.info(fullKey(IP_ADDR_KEY) + " " + IP_ADDR.getHostAddress());
		}
		logger.info("crail.storage.reflex.version" + " " + VERSION);
		logger.info(fullKey(PORT_KEY) + " " + PORT);
		logger.info(fullKey(ALLOCATION_SIZE_KEY) + " " + ALLOCATION_SIZE);
		logger.info(fullKey(NAMESPACE_SIZE_KEY) + " " + NAMESPACE_SIZE);
		logger.info(fullKey(QUEUE_SIZE_KEY) + " " + QUEUE_SIZE);
		logger.info(fullKey(BLOCK_SIZE_KEY) + " " + BLOCK_SIZE);
		logger.info(fullKey(NO_DELAY_KEY) + " " + NO_DELAY);
	}
}
