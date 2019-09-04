package com.ibm.crail.storage.reflex;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import org.apache.crail.conf.CrailConfiguration;
import org.apache.commons.cli.*;
import org.slf4j.Logger;
import org.apache.crail.storage.StorageResource;
import org.apache.crail.storage.StorageServer;
import org.apache.crail.utils.CrailUtils;

public class ReFlexStorageServer implements StorageServer {
	private static final Logger LOG = CrailUtils.getLogger();
	
	private boolean alive;
	private long namespaceSize;
	private long alignedSize;
	private long addr = 0;
	
	public ReFlexStorageServer(){
		this.alive = false;
		this.namespaceSize = ReFlexStorageConstants.NAMESPACE_SIZE; //static defaults, will update based on config in init()
		this.alignedSize = namespaceSize - (namespaceSize % ReFlexStorageConstants.ALLOCATION_SIZE);
		this.addr = 0;
		this.alive = true;
	}

	@Override
	public InetSocketAddress getAddress() {
		return new InetSocketAddress(ReFlexStorageConstants.IP_ADDR, ReFlexStorageConstants.PORT);
	}

	@Override
	public boolean isAlive() {
		return alive;
	}

	@Override
	public StorageResource allocateResource() throws Exception {
		StorageResource resource = null;
		if (alignedSize > 0){
			LOG.info("new block, length " + ReFlexStorageConstants.ALLOCATION_SIZE);
			LOG.debug("block stag 0, addr " + addr + ", length " + ReFlexStorageConstants.ALLOCATION_SIZE);
			alignedSize -= ReFlexStorageConstants.ALLOCATION_SIZE;
			addr += ReFlexStorageConstants.ALLOCATION_SIZE;
			resource = StorageResource.createResource(addr, (int)ReFlexStorageConstants.ALLOCATION_SIZE, 0);
		}
		
		return resource;
	}

	@Override
	public void init(CrailConfiguration crailConfiguration, String[] strings) throws Exception {
		ReFlexStorageConstants.updateConstants(crailConfiguration);
		this.namespaceSize = ReFlexStorageConstants.NAMESPACE_SIZE;
		this.alignedSize = namespaceSize - (namespaceSize % ReFlexStorageConstants.ALLOCATION_SIZE);

		Options options = new Options();
		Option bindIp = Option.builder("a").desc("ip address to bind to").hasArg().build();
		Option port = Option.builder("p").desc("port to bind to").hasArg().type(Number.class).build();
		//Option pcieAddress = Option.builder("s").desc("PCIe address of NVMe device").hasArg().build();
		options.addOption(bindIp);
		options.addOption(port);
		//options.addOption(pcieAddress);
		CommandLineParser parser = new DefaultParser();
		try {
			CommandLine line = parser.parse(options, strings);
			if (line.hasOption(port.getOpt())) {
				ReFlexStorageConstants.PORT = ((Number) line.getParsedOptionValue(port.getOpt())).intValue();
			}
			if (line.hasOption(bindIp.getOpt())) {
				ReFlexStorageConstants.IP_ADDR = InetAddress.getByName(line.getOptionValue(bindIp.getOpt()));
			}
		} catch (ParseException e) {
			System.err.println(e.getMessage());
			HelpFormatter formatter = new HelpFormatter();
			formatter.printHelp("ReFlex storage tier", options);
			System.exit(-1);
		}

		ReFlexStorageConstants.verify();
	}

	@Override
	public void printConf(Logger logger) {
		ReFlexStorageConstants.printConf(logger);
	}

	@Override
	public void run() {
		while(true);
	}

	public void prepareToShutDown(){

	}
}
