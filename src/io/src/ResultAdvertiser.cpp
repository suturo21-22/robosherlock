/*
 * Copyright (c) 2011, Ferenc Balint-Benczedi <balintbe@cs.uni-bremen.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Intelligent Autonomous Systems Group/
 *       Technische Universitaet Muenchen nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <uima/api.hpp>

#include <ros/ros.h>
#include <std_msgs/String.h>

#include <rs/scene_cas.h>
#include <rs/utils/time.h>

#include <rs/conversion/bson.h>


//#undef OUT_LEVEL
//#define OUT_LEVEL OUT_LEVEL_DEBUG


using namespace uima;

class ResultAdvertiser : public Annotator
{

private:
  ros::NodeHandle nh_;
  ros::Publisher resPub;

public:
  ResultAdvertiser() : nh_("~")
  {
    resPub = nh_.advertise<std_msgs::String>(std::string("result_advertiser"), 5);
  }

  TyErrorId initialize(AnnotatorContext &ctx)
  {
    outInfo("initialize");
    return UIMA_ERR_NONE;
  }

  TyErrorId destroy()
  {
    outInfo("destroy");
    return UIMA_ERR_NONE;
  }

  TyErrorId process(CAS &tcas, ResultSpecification const &res_spec)
  {
    MEASURE_TIME;
    outInfo("process begins");
    outInfo("whaa");
    rs::SceneCas cas(tcas);
    outInfo("whaa");
    rs::Scene scene = cas.getScene();
    std::vector<rs::Cluster> clusters;
    scene.identifiables.filter(clusters);

    outInfo("number of clusters: " << clusters.size());

    std::stringstream ss;
    int i = 0;
    ss << "{\"object_hypotheses:\": [";
    for(auto c : clusters)
    {
      ss << "{\"cluster\" : " << i << "," << std::endl << "\"annotations\" : [";
      rs::Cluster &cluster = c;
      int annotidx = 0;
      for(auto annotation : cluster.annotations())
      {
        mongo::BSONObj bson;
        rs::conversion::from(annotation, bson);
        ss << bson.jsonString() << (annotidx++ < cluster.annotations.size() - 1 ? "," : "");
      }
      ss << "]}" << (i++ < clusters.size() - 1 ? "," : ""); //end of annotations
    }
    ss<<"]}";
    std_msgs::String msg;
    msg.data = ss.str();
    resPub.publish(msg);
    return UIMA_ERR_NONE;
  }
};


MAKE_AE(ResultAdvertiser)
