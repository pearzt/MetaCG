/**
 * File: VersionThreeMCGWriter.cpp
 * License: Part of the MetaCG project. Licensed under BSD 3 clause license. See LICENSE.txt file at
 * https://github.com/tudasc/metacg/LICENSE.txt
 */

#include "io/VersionThreeMCGWriter.h"
#include "MCGManager.h"
#include "config.h"
#include <iostream>

void metacg::io::VersionThreeMCGWriter::write(const metacg::Callgraph* cg, metacg::io::JsonSink& js) {
  nlohmann::json j;
  attachMCGFormatHeader(j);
  j["_CG"] = *cg;
  if (exportSorted) {
    sortCallgraph(j);
  }
  if (outputDebug) {
    convertToDebug(j);
  }
  js.setJson(j);
}

void metacg::io::VersionThreeMCGWriter::sortCallgraph(nlohmann::json& j) {
  // Assume we get only the graph
  nlohmann::json* callgraph = &j;
  // If we get the whole container, extract the graph
  if (j.contains("_CG")) {
    callgraph = &j["_CG"];
  }

  for (auto& elem : *callgraph) {
    if (elem.is_array()) {
      std::sort(elem.begin(), elem.end());
    }
    // also sort the internals of the node container
    if (elem == "nodes") {
      for (auto& member : elem) {
        if (member.is_array()) {
          std::sort(member.begin(), member.end());
        }
      }
    }
  }
}

void metacg::io::VersionThreeMCGWriter::convertToDebug(nlohmann::json& json) {
  // Assume we get only the graph
  nlohmann::json* callgraph = &json;
  // If we get the whole container, extract the graph
  if (json.contains("_CG")) {
    callgraph = &json["_CG"];
  }

  std::unordered_map<size_t, const nlohmann::basic_json<>*> idNameMap;

  for (const auto& node : callgraph->at("nodes")) {
    idNameMap[node.at(0)] = &node.at(1).at("functionName");
  }

  std::unordered_map<size_t, std::vector<size_t>> callerCalleeMap;
  std::unordered_map<size_t, std::vector<size_t>> calleeCallerMap;

  for (const auto& edge : callgraph->at("edges")) {
    callerCalleeMap[edge.at(0).at(0)].push_back(edge.at(0).at(1));
    calleeCallerMap[edge.at(0).at(1)].push_back(edge.at(0).at(0));
  }

  for (auto& node : callgraph->at("nodes")) {
    node.at(1)["callees"] = nlohmann::json::array();
    if (callerCalleeMap.count(node.at(0))) {
      for (const auto& callee : callerCalleeMap.at(node.at(0))) {
        node.at(1)["callees"].push_back(*idNameMap[callee]);
      }
    }
    node.at(1)["callers"] = nlohmann::json::array();
    if (calleeCallerMap.count(node.at(0))) {
      for (const auto& caller : calleeCallerMap.at(node.at(0))) {
        node.at(1)["callers"].push_back(*idNameMap[caller]);
      }
    }
    node.at(0) = *idNameMap.at(node.at(0));
  }
}
