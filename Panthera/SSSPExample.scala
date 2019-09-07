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
package org.apache.spark.examples.graphx

// $example on$
import org.apache.spark.graphx.{Graph, VertexId}
import org.apache.spark.graphx.util.GraphGenerators
// $example off$
import org.apache.spark.sql.SparkSession
import org.apache.spark.graphx.GraphLoader
import org.apache.spark.storage.StorageLevel

/**
  * An example use the Pregel operator to express computation
  * such as single source shortest path
  * Run with
  * {{{
  * bin/run-example graphx.SSSPExample
  * }}}
  */
object SSSPExample {
  def main(args: Array[String]): Unit = {
    // Creates a SparkSession.
    val spark = SparkSession
      .builder
      .appName(s"${this.getClass.getSimpleName}")
      .getOrCreate()
    val sc = spark.sparkContext

    if (args.length < 3) {
      System.err.println("Usage:  GraphX SSSP <input dataset> <start vertex number> <partitionNum>")
      System.exit(1)
    }

    val startVertexNum = args(1).toInt
    val partitionNum = args(2).toInt

    // $example on$
    // A graph with edge attributes containing distances
    // val graph: Graph[Long, Double] =
    // GraphGenerators.logNormalGraph(sc, numVertices = 100).mapEdges(e => e.attr.toDouble)

    val graph: Graph[Int, Double] =
      GraphLoader.edgeListFile(sc, args(0),false , partitionNum, StorageLevel.MEMORY_ONLY, StorageLevel.MEMORY_ONLY)
        .mapEdges(e => e.attr.toDouble)

    var iter = 0
    var randomVal = scala.util.Random

    while (iter < startVertexNum ) {

      //val sourceId: VertexId = 42 // The ultimate source
      //Get a random start VertexId
      val sourceId: VertexId = randomVal.nextInt(999)  // Min=0, Max=999
      println("Do GraphX, iteration : %s, start vertex : %s".format(iter, sourceId))

      // Initialize the graph such that all vertices except the root have distance infinity.
      val initialGraph = graph.mapVertices((id, _) =>
        if (id == sourceId) 0.0 else Double.PositiveInfinity)

      // Debug
      // println(initialGraph.vertices.collect().mkString("\n"))
      // println(initialGraph.edges.collect().mkString("\n"))

      val sssp = initialGraph.pregel(Double.PositiveInfinity)(
        (id, dist, newDist) => math.min(dist, newDist), // Vertex Program
        triplet => { // Send Message
          if (triplet.srcAttr + triplet.attr < triplet.dstAttr) {
            Iterator((triplet.dstId, triplet.srcAttr + triplet.attr))
          } else {
            Iterator.empty
          }
        },
        (a, b) => math.min(a, b) // Merge Message
      )
      // println(sssp.vertices.collect.mkString("\n"))
      sssp.vertices.collect()
      // $example off$

      iter += 1
    }
    spark.stop()
  }
}
// scalastyle:on println
