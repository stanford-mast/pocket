#/bin/bash

kops create -f $NAME.yaml
kops create secret --name ${NAME} sshpublickey admin -i ~/.ssh/id_rsa.pub
kops update cluster ${NAME} --yes

#  Then wait. 
#  When cluster running (check with `kops validate cluster`), run the following commands:
#
# kops create instancegroup bastions --role Bastion --subnet utility-us-west-2c --name ${NAME}
# kops update cluster ${NAME} --yes
