#!/bin/bash

set -e
BASENAME=`basename $0`

SYSFS_GPIO_DIR=/sys/class/gpio
#SYSFS_GPIO_DIR=sysfsGpioEmulation

function PrintUsage {
  echo "Usage: $BASENAME port spiDeviceName nHostIntGpio nResetGpio nWakeGpio debugMask ((--nolog)|(--semaphore[-create] semaphoreName)|(nChipSelectGpio))*"
  exit 1
}

if [ $# -lt "6" ]
then
  echo "Error: too few arguments (have $#, need at least 6)"
  PrintUsage
fi

INTEGER_REGEX='^[0-9]+$'

PORT=$1
SPI_DEVICE=$2
NHOSTINT_GPIO=$3
NRESET_GPIO=$4
NWAKE_GPIO=$5
DEBUG=$6
NOLOG=
NCS_GPIO=
SEMAPHORE=

while [ $# -ge "7" ]
do
  if [ $7 = "--nolog" ]
  then
    NOLOG=$7
  else
    if [ $7 = "--semaphore" -o $7 = "--semaphore-create" ]
    then
      if [ $# -ge "8" ]
      then
        SEMAPHORE="$7 $8"
        shift
      else
        echo "Error: --semaphore[-create] needs an argument"
        PrintUsage
      fi
    else
      if ! [[ $7 =~ $INTEGER_REGEX ]]
      then
        echo "Error: invalid argument \"$7\""
        PrintUsage
      else
        NCS_GPIO=$7
      fi
    fi
  fi
  shift
done

if [ -c $SPI_DEVICE ]
then
  SPI_DEVICE_PATH=$SPI_DEVICE
else
  SPI_DEVICE_PATH=/dev/$SPI_DEVICE
  if [ ! -c $SPI_DEVICE_PATH ]
  then
    echo "spi_device_name $SPI_DEVICE does not exist"
    exit 1
  fi
fi
if [ -n "$NHOSTINT_GPIO" ]
then
  NHOSTINT_GPIO_PATH=$SYSFS_GPIO_DIR/gpio$NHOSTINT_GPIO
  if [ ! -d $NHOSTINT_GPIO_PATH ]
  then
    echo $NHOSTINT_GPIO > $SYSFS_GPIO_DIR/export
  fi
  echo in            > $NHOSTINT_GPIO_PATH/direction
  echo falling       > $NHOSTINT_GPIO_PATH/edge
fi
if [ -n "$NRESET_GPIO" ]
then
  NRESET_GPIO_PATH=$SYSFS_GPIO_DIR/gpio$NRESET_GPIO
  if [ ! -d $NRESET_GPIO_PATH ]
  then
    echo $NRESET_GPIO > $SYSFS_GPIO_DIR/export
  fi
  echo high          > $NRESET_GPIO_PATH/direction
fi
if [ -n "$NWAKE_GPIO" ]
then
  NWAKE_GPIO_PATH=$SYSFS_GPIO_DIR/gpio$NWAKE_GPIO
  if [ ! -d $NWAKE_GPIO_PATH ]
  then
    echo $NWAKE_GPIO > $SYSFS_GPIO_DIR/export
  fi
  echo high          > $NWAKE_GPIO_PATH/direction
fi
if [ -n "$NCS_GPIO" ]
then
  NCS_GPIO_PATH=$SYSFS_GPIO_DIR/gpio$NCS_GPIO
  if [ ! -d $NCS_GPIO_PATH ]
  then
    echo $NCS_GPIO > $SYSFS_GPIO_DIR/export
  fi
  echo high          > $NCS_GPIO_PATH/direction
fi

CMDLINE="./spi-server -p $PORT -s $SPI_DEVICE_PATH -i gpio$NHOSTINT_GPIO\
 -r gpio$NRESET_GPIO -w gpio$NWAKE_GPIO -T $DEBUG $NOLOG $SEMAPHORE"

if [ -n "$NCS_GPIO" ]
then
  CMDLINE+=" -c gpio$NCS_GPIO"
fi

echo "$CMDLINE"
$CMDLINE &
BG_PID=$!
sleep 2
if ! kill -0 $BG_PID > /dev/null 2>&1
then
  echo "Failed to start SPI Server"
  exit 1
else
  echo "SPI Server is pid $BG_PID"
fi
