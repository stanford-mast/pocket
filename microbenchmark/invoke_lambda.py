""" Module to Invoke Lambda. """
import boto3
import botocore
import argparse

client = boto3.client('lambda')
payload = ""
function_name = "pocket_latency_test"

def invoke_lambda():
  try:
      response = client.invoke(FunctionName=function_name,
                           InvocationType='Event', Payload=payload)
      
      if response['StatusCode'] != 202:
          print('Error Invoking Lambda.')

  except Exception as e:
      print("Error Invoking Lambda {} from AWS client.".format(function_name))
      raise e

if __name__ == '__main__':
    invoke_lambda()
