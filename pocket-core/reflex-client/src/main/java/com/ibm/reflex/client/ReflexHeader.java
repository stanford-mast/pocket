package com.ibm.reflex.client;

import java.nio.ByteBuffer;

//typedef struct __attribute__ ((__packed__)) {
//	  uint16_t magic;
//	  uint16_t opcode;
//	  void *req_handle;
//	  unsigned long lba;
//	  unsigned int lba_count;
//} binary_header_blk_t;

public class ReflexHeader {
	public static final int HEADERSIZE = Short.BYTES + Short.BYTES + Long.BYTES + Long.BYTES + Integer.BYTES;
	
	private short magic;
	private short type;
	private long ticket;
	private long lba;
	private int count;
	
	public ReflexHeader(){
		this.magic = 0;
		this.type = 0;
		this.ticket = 0; 
		this.lba = 0;
		this.count = 0;
	}
	
	public void update(ByteBuffer buffer){
		this.magic = buffer.getShort();
		this.type = buffer.getShort();
		this.ticket = buffer.getLong();
		this.lba = buffer.getLong();
		this.count = buffer.getInt();		
	}

	public short getMagic() {
		return magic;
	}

	public short getType() {
		return type;
	}

	public long getTicket() {
		return ticket;
	}

	public long getLba() {
		return lba;
	}

	public int getCount() {
		return count;
	}
}
