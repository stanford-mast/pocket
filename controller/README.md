## Pocket controller

To use the Pocket controller, you must first launch VMs for your Pocket cluster. The controller
will scale containers based on job requirements and resource usage. See the 
[deploy README](https://github.com/stanford-mast/pocket/blob/master/deploy/README.md) for
instrucitons about setting up a Pocket cluster. 

Before using the controller, make sure to copy the `pocket.py` file from the clientlib directory to the
controller folder. Use python3 to run the controller as it relies on the asyncio module. 

```
./fetch-deps.sh
python3 controller.py
```
