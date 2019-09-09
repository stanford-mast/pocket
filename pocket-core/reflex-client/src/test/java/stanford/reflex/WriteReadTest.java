package stanford.reflex;

import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.util.Arrays;
import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.slf4j.Logger;

import com.ibm.reflex.client.ReflexClientGroup;
import com.ibm.reflex.client.ReflexEndpoint;
import com.ibm.reflex.client.ReflexUtils;

public class WriteReadTest  implements Runnable {
	private static final Logger LOG = ReflexUtils.getLogger();
	
	private ReflexEndpoint endpoint;
	private int loopCount;
	
	public WriteReadTest(ReflexEndpoint endpoint, int queueDepth, int batchCount, int loopCount, int size) throws InterruptedException{
		this.endpoint = endpoint;
		this.loopCount = loopCount;
	}

	public void run() {
		try {
			LOG.info("WriteReadTest 1.0, loopCount " + loopCount);
			ByteBuffer buffer = ByteBuffer.allocate(endpoint.getGroup().getBlockSize());
			for (int i = 0; i < loopCount; i++){
				buffer.clear();
				fillBuffer(buffer, i);
				LOG.info("write value " + i);
				buffer.clear();
				endpoint.put(i, buffer).get();
			}
			for (int i = 0; i < loopCount; i++){
				buffer.clear();
				endpoint.get(i, buffer).get();
				buffer.clear();
				checkBuffer(buffer, i);
				LOG.info("read value " + i);
			}			
		} catch(Exception e){
			e.printStackTrace();
		}
	}
	
	private void fillBuffer(ByteBuffer buffer, int value) {
		int count = buffer.remaining() / Integer.BYTES;
		for (int i = 0; i < count; i++){
			buffer.putInt(value);
		}
	}

	private void checkBuffer(ByteBuffer buffer, int value) throws Exception {
		int count = buffer.remaining() / Integer.BYTES;
		for (int i = 0; i < count; i++){
			int actualValue = buffer.getInt();
			if (actualValue != value){
				throw new Exception("Read value " + actualValue + ", expectedValue " + value);
			}
		}
	}

	public static void main(String[] args) throws Exception{
		int queueDepth = ReflexClientGroup.DEFAULT_QUEUE_DEPTH;
		int loop = queueDepth;
		int batchCount = queueDepth;
		String ipAddress = "localhost";
		int port = 1234;
		int size = ReflexClientGroup.DEFAULT_BLOCK_SIZE;
		boolean noDelay = false;
		
		if (args != null) {
			Option queueOption = Option.builder("q").desc("queue length").hasArg().build();
			Option loopOption = Option.builder("k").desc("loop").hasArg().build();
			Option batchOption = Option.builder("b").desc("batch of RPCs").hasArg().build();
			Option addressOption = Option.builder("a").desc("address of reflex server").hasArg().build();
			Option portOption = Option.builder("p").desc("port of reflex server").hasArg().build();
			Option sizeOption = Option.builder("s").desc("size").hasArg().build();
			Option noDelayOption = Option.builder("d").desc("nodelay").hasArg().build();
			Options options = new Options();
			options.addOption(queueOption);
			options.addOption(loopOption);
			options.addOption(batchOption);
			options.addOption(addressOption);
			options.addOption(portOption);
			options.addOption(sizeOption);
			options.addOption(noDelayOption);
			CommandLineParser parser = new DefaultParser();

			try {
				CommandLine line = parser.parse(options, Arrays.copyOfRange(args, 0, args.length));
				if (line.hasOption(queueOption.getOpt())) {
					queueDepth = Integer.parseInt(line.getOptionValue(queueOption.getOpt()));
				}
				if (line.hasOption(loopOption.getOpt())) {
					loop = Integer.parseInt(line.getOptionValue(loopOption.getOpt()));
				}	
				if (line.hasOption(batchOption.getOpt())) {
					batchCount = Integer.parseInt(line.getOptionValue(batchOption.getOpt()));
				}	
				
				if (line.hasOption(addressOption.getOpt())) {
					ipAddress = line.getOptionValue(addressOption.getOpt());
				}
				if (line.hasOption(portOption.getOpt())) {
					port = Integer.parseInt(line.getOptionValue(portOption.getOpt()));
				}		
				if (line.hasOption(sizeOption.getOpt())) {
					size = Integer.parseInt(line.getOptionValue(sizeOption.getOpt()));
				}		
				if (line.hasOption(noDelayOption.getOpt())) {
					noDelay = Boolean.parseBoolean(line.getOptionValue(noDelayOption.getOpt()));
				}				
			} catch (ParseException e) {
				HelpFormatter formatter = new HelpFormatter();
				formatter.printHelp("TCP RPC", options);
				System.exit(-1);
			}
		}	
		
		ReflexClientGroup clientGroup = new ReflexClientGroup(queueDepth, ReflexClientGroup.DEFAULT_BLOCK_SIZE, noDelay);
		ReflexEndpoint endpoint = clientGroup.createEndpoint();
		InetSocketAddress address = new InetSocketAddress(ipAddress, port);
		endpoint.connect(address);	
		WriteReadTest client = new WriteReadTest(endpoint, queueDepth, batchCount, loop, size);
		client.run();
	}
}
