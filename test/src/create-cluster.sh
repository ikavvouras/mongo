#!/bin/bash

RESOURCES=../resources
MONGO_PATH=~/ClionProjects/mongo

MONGO=${MONGO_PATH}/mongo
MONGOD=${MONGO_PATH}/mongod
MONGOS=${MONGO_PATH}/mongos

echo 'starting mongo-config'
rm -rf $RESOURCES/mongo-metadata
mkdir $RESOURCES/mongo-metadata
#$MONGOD --configsvr --replSet norepl --dbpath $RESOURCES/mongo-metadata --port 27017 > config.log &
$MONGOD --configsvr --dbpath $RESOURCES/mongo-metadata --port 27017 > config.log &

sleep 2

echo 'starting mongo-router'
rm -rf $RESOURCES/mongo-router
mkdir $RESOURCES/mongo-router
#$MONGOS --configdb norepl/localhost:27017 --port 27018 > router.log &
$MONGOS --configdb localhost:27017 --port 27018 > router.log &

sleep 2

echo 'starting mongo-shard-1'
rm -rf $RESOURCES/mongo-shard1
mkdir $RESOURCES/mongo-shard1
$MONGOD --dbpath $RESOURCES/mongo-shard1 --port 27019 > shard1.log &

echo 'starting mongo-shard-2'
rm -rf $RESOURCES/mongo-shard2
mkdir $RESOURCES/mongo-shard2
$MONGOD --dbpath $RESOURCES/mongo-shard2 --port 27020 > shard2.log &

sleep 2

echo 'configure cluster'
echo 'sh.addShard( "localhost:27019" ) ; sh.addShard( "localhost:27020" ) ; exit' > $MONGO --quiet localhost:27018

echo 'configure sharding for test_db'
commands="use test_db;
sh.enableSharding(\"test_db\")'
db.test_collection.ensureIndex( { _id : \"hashed\" } );
sh.shardCollection(\"test_db.test_collection\", { \"_id\": \"hashed\" } );
sh.status();
exit;
"
echo $commands > $MONGO --quiet localhost:27018
