import subprocess as sp
import shlex
from subprocess import Popen, PIPE
import re
import time


NAMENODE_IP = "10.1.191.110"

def get_exitcode_stdout_stderr(cmd):
    """
    Execute the external command and get its exitcode, stdout and stderr.
    """
    args = shlex.split(cmd)

    proc = Popen(args, stdout=PIPE, stderr=PIPE)
    out, err = proc.communicate()
    exitcode = proc.returncode
    #
    return exitcode, out, err


def remove_i3_enis():
    # get attachment id
    cmd = "aws ec2 describe-network-interfaces --filter Name=description,Values='*eni for i3*' --query \"NetworkInterfaces[*].Attachment.AttachmentId\""
    exitcode, out, err = get_exitcode_stdout_stderr(cmd)
    pattern = r'"([A-Za-z0-9_\./\\-]*)"'
    p = re.compile(pattern)
    i3_attachment_ids = p.findall(out) 
    print "i3 attachement ids: ", i3_attachment_ids

    for i3_attachment_id in i3_attachment_ids:
        # detach eni
        cmd = "aws ec2 detach-network-interface --attachment-id " + i3_attachment_id
        sp.call(cmd, shell=True)
    
    # get i3 eni id
    cmd = "aws ec2 describe-network-interfaces --filter Name=description,Values='*eni for i3*' --query \"NetworkInterfaces[*].NetworkInterfaceId\""
    exitcode, out, err = get_exitcode_stdout_stderr(cmd)
    i3_enis = p.findall(out) 
    print "i3 enis: ", i3_enis
    
    time.sleep(60)

    for i3_eni in i3_enis:
        # delete eni to free up security group for kops to delete
        cmd = "aws ec2 delete-network-interface --network-interface-id " + i3_eni
        sp.call(cmd, shell=True)

def remove_h1_enis():
    # get attachment id
    cmd = "aws ec2 describe-network-interfaces --filter Name=description,Values='*eni for h1*' --query \"NetworkInterfaces[*].Attachment.AttachmentId\""
    exitcode, out, err = get_exitcode_stdout_stderr(cmd)
    pattern = r'"([A-Za-z0-9_\./\\-]*)"'
    p = re.compile(pattern)
    h1_attachment_ids = p.findall(out) 
    print "h1 attachement ids: ", h1_attachment_ids

    for h1_attachment_id in h1_attachment_ids:
        # detach eni
        cmd = "aws ec2 detach-network-interface --attachment-id " + h1_attachment_id
        sp.call(cmd, shell=True)
    
    # get h1 eni id
    cmd = "aws ec2 describe-network-interfaces --filter Name=description,Values='*eni for h1*' --query \"NetworkInterfaces[*].NetworkInterfaceId\""
    exitcode, out, err = get_exitcode_stdout_stderr(cmd)
    h1_enis = p.findall(out) 
    print "h1 enis: ", h1_enis
    
    time.sleep(60)

    for h1_eni in h1_enis:
        # delete eni to free up security group for kops to delete
        cmd = "aws ec2 delete-network-interface --network-interface-id " + h1_eni
        sp.call(cmd, shell=True)

def remove_i2_enis():
    # get attachment id
    cmd = "aws ec2 describe-network-interfaces --filter Name=description,Values='*eni for i2*' --query \"NetworkInterfaces[*].Attachment.AttachmentId\""
    exitcode, out, err = get_exitcode_stdout_stderr(cmd)
    pattern = r'"([A-Za-z0-9_\./\\-]*)"'
    p = re.compile(pattern)
    i2_attachment_ids = p.findall(out) 
    print "i2 attachement ids: ", i2_attachment_ids

    for i2_attachment_id in i2_attachment_ids:
        # detach eni
        cmd = "aws ec2 detach-network-interface --attachment-id " + i2_attachment_id
        sp.call(cmd, shell=True)
    
    # get i2 eni id
    cmd = "aws ec2 describe-network-interfaces --filter Name=description,Values='*eni for i2*' --query \"NetworkInterfaces[*].NetworkInterfaceId\""
    exitcode, out, err = get_exitcode_stdout_stderr(cmd)
    i2_enis = p.findall(out) 
    print "i2 enis: ", i2_enis
    
    time.sleep(60)

    for i2_eni in i2_enis:
        # delete eni to free up security group for kops to delete
        cmd = "aws ec2 delete-network-interface --network-interface-id " + i2_eni
        sp.call(cmd, shell=True)
def remove_namenode_eni():
    # get attachment id
    cmd = "aws ec2 describe-network-interfaces --filter Name=description,Values='*eni for namenode*' --query \"NetworkInterfaces[*].Attachment.AttachmentId\""
    exitcode, out, err = get_exitcode_stdout_stderr(cmd)
    pattern = r'"([A-Za-z0-9_\./\\-]*)"'
    attachment_id = re.search(pattern, out).group().strip('\"')

    # detach eni
    cmd = "aws ec2 detach-network-interface --attachment-id " + attachment_id
    sp.call(cmd, shell=True)

    # get namenode eni id
    cmd = "aws ec2 describe-network-interfaces --filter Name=description,Values='*eni for namenode*' --query \"NetworkInterfaces[*].NetworkInterfaceId\""
    exitcode, out, err = get_exitcode_stdout_stderr(cmd)
    pattern = r'"([A-Za-z0-9_\./\\-]*)"'
    namenode_eni = re.search(pattern, out).group().strip('\"')
    print "Namenode ENI id is: " + namenode_eni

    time.sleep(60)

    # delete eni to free up security group for kops to delete
    cmd = "aws ec2 delete-network-interface --network-interface-id " + namenode_eni
    sp.call(cmd, shell=True)
    
def main():
    remove_namenode_eni()
    remove_i3_enis()
    remove_h1_enis()
    remove_i2_enis()


if __name__ == '__main__':
    main()
