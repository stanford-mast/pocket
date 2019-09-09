## Pocket controller

To use the Pocket controller, you must first launch VMs for your Pocket cluster. The controller
will scale containers based on job requirements and resource usage. See the 
[deploy README](https://github.com/stanford-mast/pocket/blob/master/deploy/README.md) for
instrucitons about setting up a Pocket cluster. 

Before using the controller, make sure to copy the `pocket.py` file from the client directory to the
controller folder. Use python3 to run the controller as it relies on the asyncio module. 

```
cp ../client/pocket.py .
cp ../client/build/pocket/libpocket.so .
cp ../client/build/client/libcppcrail.so .
python3 controller.py
```
