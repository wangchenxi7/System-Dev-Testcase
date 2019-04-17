/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// scalastyle:off println
package org.apache.spark.examples

import scala.collection.mutable
import scala.util.Random

import org.apache.spark.sql.SparkSession

/**
  * Transitive closure on a graph.
  */
object SparkTC {

  /*
  val numEdges = 200
  val numVertices = 100
  val rand = new Random(42)


  def generateGraph: Seq[(Int, Int)] = {
    val edges: mutable.Set[(Int, Int)] = mutable.Set.empty
    while (edges.size < numEdges) {
      val from = rand.nextInt(numVertices)
      val to = rand.nextInt(numVertices)
      if (from != to) edges.+=((from, to))
    }
    edges.toSeq
  }
  */

  def main(args: Array[String]) {
    val spark = SparkSession
      .builder
      .appName("SparkTC")
      .getOrCreate()

    if (args.length < 1) {
      System.err.println("Usage: Spark Transitive Closure <file> <iter> <partition number>")
      System.exit(1)
    }

    val iteration = if (args.length > 1) args(1).toInt else 2
    val slices = if (args.length > 2) args(2).toInt else 8
    // var tc = spark.sparkContext.parallelize(generateGraph, slices).cache()

    // load data from hdfs
    val input = spark.sparkContext.textFile(args(0), slices)
    var tc = input.map { s =>
      val parts = s.split("\\s+")
      (parts(0), parts(1))
    }.distinct().cache()

    // Linear transitive closure: each round grows paths by one edge,
    // by joining the graph's edges with the already-discovered paths.
    // e.g. join the path (y, z) from the TC with the edge (x, y) from
    // the graph to obtain the path (x, z).

    // Because join() joins on keys, the edges are stored in reversed order.
    val edges = tc.map(x => (x._2, x._1))

    // This join is iterated until a fixed point is reached.
    var oldCount = 0L
    var iterControl = 1L  // Check at the end of excution, so start from 1.
    var continure = true

    var nextCount = tc.count()  // action, Job0 finish
    do {
      oldCount = nextCount
      // Perform the join, obtaining an RDD of (y, (z, x)) pairs,
      // then project the result to obtain the new (x, z) paths.
      tc = tc.union(tc.join(edges).map(x => (x._2._2, x._2._1))).distinct().cache()   // distinct 不会减少 partition 数目？
      nextCount = tc.count()    // action

      // debug
      println(" iteration : " + iterControl + " finished; current count: " + nextCount.toString + "; " +
                                                                      " last count :" +oldCount.toString)


      if(iterControl == iteration || nextCount == oldCount){
        continure = false
      }

      iterControl += 1
    } while (continure)

    println("TC has " + tc.count() + " edges.")
    spark.stop()
  }

}
// scalastyle:on println
