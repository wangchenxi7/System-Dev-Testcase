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

import java.util.Random

import scala.math.exp

import breeze.linalg.{DenseVector, Vector}

import org.apache.spark.sql.SparkSession

/**
  * Logistic regression based classification.
  * Usage: SparkLR [slices]
  *
  * This is an example implementation for learning how to use Spark. For more conventional use,
  * please refer to org.apache.spark.ml.classification.LogisticRegression.
  */
object SparkLR {
  // val N = 100  // Number of data points
  val D = 2   // Number of dimensions, for wikipedia, it's 2
  val R = 0.7  // Scaling factor
  // val ITERATIONS = 5
  val rand = new Random(42)

  case class DataPoint(x: Vector[Double], y: Double)

  /*
  def generateData: Array[DataPoint] = {
    def generatePoint(i: Int): DataPoint = {
      val y = if (i % 2 == 0) -1 else 1
      val x = DenseVector.fill(D) {rand.nextGaussian + y * R}
      DataPoint(x, y)
    }
    Array.tabulate(N)(generatePoint)
  }
  */

  def parseVector(line: String): DataPoint = {
    val y = scala.util.Random.nextInt(2)  // y can be [0,2) -> 0 or 1
    val x = DenseVector(line.split("\\s+").map(_.toDouble + y* R) )
    DataPoint(x,y)
  }




  def showWarning() {
    System.err.println(
      """WARN: This is a naive implementation of Logistic Regression and is given as an example!
        |Please use org.apache.spark.ml.classification.LogisticRegression
        |for more conventional use.
      """.stripMargin)
  }

  def main(args: Array[String]) {

    showWarning()

    val spark = SparkSession
      .builder
      .appName("SparkLR")
      .getOrCreate()

    if (args.length < 3){
      System.err.println( " Usage: SparkLR <file> <iter> <partitionNum>")
      System.exit(1)
    }

    val iteration = args(1).toInt
    val numSlices = if (args.length >= 3) args(2).toInt else 8

    //val points = spark.sparkContext.parallelize(generateData, numSlices).cache()
    val points = spark.sparkContext.textFile(args(0), numSlices).map(parseVector _).cache()

    // Initialize w to a random value
    var w = DenseVector.fill(D) {2 * rand.nextDouble - 1}
    println("Initial w: " + w)

    for (i <- 1 to iteration) {
      println("On iteration " + i)
      val gradient = points.map { p =>
        p.x * (1 / (1 + exp(-p.y * (w.dot(p.x)))) - 1) * p.y
      }.reduce(_ + _)
      w -= gradient
    }

    println("Final w: " + w)

    spark.stop()
  }
}
// scalastyle:on println
