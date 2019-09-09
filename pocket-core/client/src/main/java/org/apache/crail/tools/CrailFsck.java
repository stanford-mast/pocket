/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.crail.tools;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.concurrent.atomic.AtomicInteger;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.crail.*;
import org.apache.crail.conf.CrailConfiguration;
import org.apache.crail.conf.CrailConstants;
import org.apache.crail.core.CoreDataStore;
import org.apache.crail.core.DirectoryInputStream;
import org.apache.crail.core.DirectoryRecord;
import org.apache.crail.metadata.DataNodeInfo;
import org.apache.crail.metadata.FileName;
import org.apache.crail.rpc.IOCtlCommand;
import org.apache.crail.utils.CrailUtils;


public class CrailFsck {
	
	public CrailFsck(){
		
	}
	
	public void getLocations(String filename, long offset, long length) throws Exception {
		System.out.println("getLocations, filename " + filename + ", offset " + offset + ", len " + length);
		CrailConfiguration conf = new CrailConfiguration();
		CrailStore fs = CrailStore.newInstance(conf);
		
		CrailBlockLocation locations[] = fs.lookup(filename).get().getBlockLocations(offset, length);
		for (int i = 0; i < locations.length; i++){
			System.out.println("location " + i + " : " + locations[i].toString());
		}	
		fs.close();
	}
	
	public void blockStatistics(String filename) throws Exception {
		HashMap<String, AtomicInteger> stats = new HashMap<String, AtomicInteger>();
		CrailConfiguration conf = new CrailConfiguration();
		CrailStore fs = CrailStore.newInstance(conf);
		CrailNode node = fs.lookup(filename).get();
		
		if (node.getType() == CrailNodeType.DIRECTORY){
			CrailDirectory directory = node.asDirectory();
			Iterator<String> iter = directory.listEntries();
			while (iter.hasNext()) {
				String path = iter.next();
				CrailFile child = fs.lookup(path).get().asFile();
				walkBlocks(stats, fs, child.getPath(), 0, child.getCapacity());
			}
		} else if (node.getType() == CrailNodeType.DATAFILE){
			CrailFile file = node.asFile();
			walkBlocks(stats, fs, file.getPath(), 0, file.getCapacity());
		} else if (node.getType() == CrailNodeType.MULTIFILE){
			CrailMultiFile directory = node.asMultiFile();
			Iterator<String> iter = directory.listEntries();
			while (iter.hasNext()) {
				String path = iter.next();
				CrailFile child = fs.lookup(path).get().asFile();
				walkBlocks(stats, fs, child.getPath(), 0, child.getCapacity());
			}
		}
		
		printStats(stats);	
		fs.close();
	}

	public void namenodeDump()  throws Exception {
		CrailConfiguration conf = new CrailConfiguration();
		CoreDataStore fs = new CoreDataStore(conf);
		for (int i = 0; i < CrailUtils.getNameNodeList().size(); i++) {
			fs.dumpNameNode();
		}
		fs.close();
	}

	public void directoryDump(String filename, boolean randomize) throws Exception {
		CrailConfiguration conf = new CrailConfiguration();
		CrailConstants.updateConstants(conf);
		CoreDataStore fs = new CoreDataStore(conf);		
		DirectoryInputStream iter = fs._listEntries(filename, randomize);
		System.out.println("#hash   \t\tname\t\tfilecomponent");
		int i = 0;
		while(iter.hasRecord()){
			DirectoryRecord record = iter.nextRecord();
			String path = CrailUtils.combinePath(record.getParent(), record.getFile());
			FileName hash = new FileName(path);
			System.out.format(i + ": " + "%08d\t\t%s\t%d\n", record.isValid() ? 1 : 0, padRight(record.getFile(), 8), hash.getFileComponent());
			i++;
		}
		iter.close();
		fs.closeFileSystem();
	}
	
	public void ping() throws Exception {
		CrailConfiguration conf = new CrailConfiguration();
		CrailConstants.updateConstants(conf);
		CoreDataStore fs = new CoreDataStore(conf);
		fs.ping();
		fs.closeFileSystem();		
	}

	public void IOCtlRemoveDN(InetAddress datanode, int port) throws Exception {
		CrailConfiguration conf = new CrailConfiguration();
		CrailConstants.updateConstants(conf);
		CoreDataStore fs = new CoreDataStore(conf);
		IOCtlCommand.RemoveDataNode cmd = new IOCtlCommand.RemoveDataNode(datanode, port);
		fs.ioctlNameNode(cmd);
		System.out.println("Datanode at : " + cmd.getIPAddress() + "/port: " + port + " scheduled for removal successfully");
		fs.closeFileSystem();
	}

	public void IOCtlGetClassStats(int storageClass) throws Exception {
		CrailConfiguration conf = new CrailConfiguration();
		CrailConstants.updateConstants(conf);
		CoreDataStore fs = new CoreDataStore(conf);
		IOCtlCommand.GetClassStatCommand cmd = new IOCtlCommand.GetClassStatCommand(storageClass);
		IOCtlResponse.GetClassStatResp stats = (IOCtlResponse.GetClassStatResp) fs.ioctlNameNode(cmd);
		System.out.println(stats);
		fs.closeFileSystem();
	}

	public void testWMask(String dirname, String wmaskArgs) throws Exception {
		CrailConfiguration conf = new CrailConfiguration();
		CrailConstants.updateConstants(conf);
		CoreDataStore fs = new CoreDataStore(conf);
		WeightMask mask = makeMask(wmaskArgs);
		IOCtlCommand.AttachWeigthMaskCommand cmd = new IOCtlCommand.AttachWeigthMaskCommand(new FileName(dirname), mask);
		IOCtlResponse.IOCtlVoidResp resp = (IOCtlResponse.IOCtlVoidResp) fs.ioctlNameNode(cmd);
		System.err.println("Void return code was: "+ resp.ioctlErrorCode());
		fs.closeFileSystem();
	}

	public void countFiles(String dirname) throws Exception {
		CrailConfiguration conf = new CrailConfiguration();
		CrailConstants.updateConstants(conf);
		CoreDataStore fs = new CoreDataStore(conf);
		IOCtlCommand.CountFilesCommand cmd = new IOCtlCommand.CountFilesCommand (new FileName(dirname));
		IOCtlResponse.CountFilesResp resp = (IOCtlResponse.CountFilesResp) fs.ioctlNameNode(cmd);
		System.err.println("Number of files at " + dirname + " are > " + resp);
		fs.closeFileSystem();
	}

	public void createDirectory(String filename, int storageClass, int locationClass) throws Exception {
		System.out.println("createDirectory, filename " + filename + ", storageClass " + storageClass + ", locationClass " + locationClass);
		CrailConfiguration conf = new CrailConfiguration();
		CrailStore fs = CrailStore.newInstance(conf);
		fs.create(filename, CrailNodeType.DIRECTORY, CrailStorageClass.get(storageClass), CrailLocationClass.get(locationClass), true).get().syncDir();
		fs.close();
	}	
	
	//-----------------

	private WeightMask makeMask(String string) throws IllegalArgumentException, UnknownHostException {
		WeightMask mask = new WeightMask();
		if(string == null){
			return mask;
		}
		String[] tokens = string.split(",");
		if(tokens.length % 3 != 0){
			throw new IllegalArgumentException(" tokens are : " + tokens.length + " must be a multiple of 3");
		}
		for(int i = 0; i < tokens.length; i+=1){
			tokens[i] = tokens[i].trim();
		}

		for(int i = 0; i < tokens.length; i+=3){
			DataNodeWeight w = new DataNodeWeight();
			byte[] ip = InetAddress.getByName(tokens[i]).getAddress();
			int port = 50020; // default port number
			if(!tokens[i + 2].isEmpty()){
				port = Integer.parseInt(tokens[i+2]);
			}
			float weight = Float.parseFloat(tokens[i+1]);
			w.datanodeHash = DataNodeInfo.calcDataNodeKey(ip, port);
			w.weight = weight;
			mask.addMask(w);
		}
		return mask;
	}

	private String padRight(String s, int n) {
		return String.format("%1$-" + n + "s", s);
	}
	
	private void printStats(HashMap<String, AtomicInteger> stats) {
		for (Iterator<String> iter = stats.keySet().iterator(); iter.hasNext(); ){
			String key = iter.next();
			System.out.println(key + "\t" + stats.get(key));
		}
	}

	private void walkBlocks(HashMap<String, AtomicInteger> stats, CrailStore fs, String filePath, long offset, long len) throws Exception {
//		System.out.println("printing locations for path " + filePath);
		CrailBlockLocation locations[] = fs.lookup(filePath).get().asFile().getBlockLocations(offset, len);
		for (int i = 0; i < locations.length; i++){
			for (int j = 0; j < locations[i].getNames().length; j++){
				String name = locations[i].getNames()[j];
				String host = name.split(":")[0];
//				System.out.println("..........names " + host);
				incStats(stats, host);
			}
		}
	}
	
	private void incStats(HashMap<String, AtomicInteger> stats, String host) {
		if (!stats.containsKey(host)){
			stats.put(host, new AtomicInteger(0));
		}
		stats.get(host).incrementAndGet();
	}	

	
	public static void main(String[] args) throws Exception {
		InetAddress datanodeAddress = null;
		int port = 50020; // the default TCP port number
		String type = "";
		String filename = "/tmp.dat";
		String maskString = null;
		long offset = 0;
		long length = 1;
		boolean randomize = false;	
		int storageClass = 0;
		int locationClass = 0;		
		
		Option typeOption = Option.builder("t").desc("type of experiment [getLocations|directoryDump|namenodeDump|blockStatistics|ping|createDirectory|removeDataNode|getClassStats|testWMask|countFiles]").hasArg().build();
		Option dataNodeOption = Option.builder("d").desc("datanode to be removed").hasArg().build();
		Option fileOption = Option.builder("f").desc("filename").hasArg().build();
		Option offsetOption = Option.builder("y").desc("offset into the file").hasArg().build();
		Option lengthOption = Option.builder("l").desc("length of the file [bytes]").hasArg().build();
		Option storageOption = Option.builder("c").desc("storageClass for file [1..n]").hasArg().build();
		Option locationOption = Option.builder("p").desc("locationClass for file [1..n]").hasArg().build();
		Option portOption = Option.builder("P").desc("port of the datanode to eject").hasArg().build();
		Option maskOption = Option.builder("m").desc("weigthmask: a sequence of <hostname,weight,port>, like localhost,0.2,5001,localhost,0.5,,localhost,0.3,5002. Args can be skipped but not the comma").hasArg().build();
		Option helpOption = Option.builder("h").desc("show help").build();
		
		Options options = new Options();
		options.addOption(dataNodeOption);
		options.addOption(typeOption);
		options.addOption(fileOption);
		options.addOption(offsetOption);
		options.addOption(lengthOption);
		options.addOption(storageOption);
		options.addOption(locationOption);
		options.addOption(maskOption);
		options.addOption(helpOption);
		options.addOption(portOption);
		
		CommandLineParser parser = new DefaultParser();
		CommandLine line = parser.parse(options, Arrays.copyOfRange(args, 0, args.length));
		if (line.hasOption(typeOption.getOpt())) {
			type = line.getOptionValue(typeOption.getOpt());
		}
		if (line.hasOption(fileOption.getOpt())) {
			filename = line.getOptionValue(fileOption.getOpt());
		}
		if (line.hasOption(dataNodeOption.getOpt())) {
			datanodeAddress = InetAddress.getByName(line.getOptionValue(dataNodeOption.getOpt()));
		}
		if (line.hasOption(portOption.getOpt())) {
			port = Integer.parseInt(line.getOptionValue(portOption.getOpt()));
		}
		if (line.hasOption(offsetOption.getOpt())) {
			offset = Long.parseLong(line.getOptionValue(offsetOption.getOpt()));
		}
		if (line.hasOption(lengthOption.getOpt())) {
			length = Long.parseLong(line.getOptionValue(lengthOption.getOpt()));
		}
		if (line.hasOption(storageOption.getOpt())) {
			storageClass = Integer.parseInt(line.getOptionValue(storageOption.getOpt()));
		}
		if (line.hasOption(locationOption.getOpt())) {
			locationClass = Integer.parseInt(line.getOptionValue(locationOption.getOpt()));
		}
		if (line.hasOption(maskOption.getOpt())) {
			maskString = line.getOptionValue(maskOption.getOpt());
		}
		if(line.hasOption(helpOption.getOpt())){
			HelpFormatter formatter = new HelpFormatter();
			formatter.printHelp("crail fsck", options);
			System.exit(0);
		}
		
		CrailFsck fsck = new CrailFsck();
		if (type.equals("getLocations")){
			fsck.getLocations(filename, offset, length);
		} else if (type.equals("directoryDump")){
			fsck.directoryDump(filename, randomize);
		} else if (type.equals("namenodeDump")){
			fsck.namenodeDump();
		} else if (type.equals("blockStatistics")){
			fsck.blockStatistics(filename);
		} else if (type.equals("ping")){
			fsck.ping();
		} else if (type.equals("createDirectory")){
			fsck.createDirectory(filename, storageClass, locationClass);
		} else if (type.equals("removeDataNode")){
			fsck.IOCtlRemoveDN(datanodeAddress, port);
		} else if (type.equals("getClassStats")){
			fsck.IOCtlGetClassStats(storageClass);
		} else if (type.equals("testWMask")){
			fsck.testWMask(filename, maskString);
		} else if (type.equals("countFiles")){
			fsck.countFiles(filename);
		}
		else {
			HelpFormatter formatter = new HelpFormatter();
			formatter.printHelp("crail fsck", options);
			System.exit(-1);	
		}
	}
}
