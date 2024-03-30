/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "gui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include <algorithm>

String sysDefID;

void FurnaceGUI::drawSysDefs(std::vector<FurnaceGUISysDef>& category, bool& accepted, std::vector<int>& sysDefStack) {
  int index=0;
  String sysDefIDLeader="##NS";
  for (int i: sysDefStack) {
    sysDefIDLeader+=fmt::sprintf("/%d",i);
  }
  for (FurnaceGUISysDef& i: category) {
    bool treeNode=false;
    bool isHovered=false;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (!i.subDefs.empty()) {
      if (i.orig.empty()) {
        sysDefID=fmt::sprintf("%s%s/%dS",i.name,sysDefIDLeader,index);
      } else {
        sysDefID=fmt::sprintf("%s/%dS",sysDefIDLeader,index);
      }
      treeNode=ImGui::TreeNodeEx(sysDefID.c_str(),i.orig.empty()?ImGuiTreeNodeFlags_SpanAvailWidth:0);
      ImGui::SameLine();
    }
    if (!i.orig.empty()) {
      sysDefID=fmt::sprintf("%s%s/%d",i.name,sysDefIDLeader,index);
      if (ImGui::Selectable(sysDefID.c_str(),false,ImGuiSelectableFlags_DontClosePopups)) {
        nextDesc=i.definition;
        nextDescName=i.name;
        accepted=true;
      }
      if (ImGui::IsItemHovered()) isHovered=true;
    } else if (i.subDefs.empty()) {
      ImGui::TextUnformatted(i.name.c_str());
      if (ImGui::IsItemHovered()) isHovered=true;
    }
    if (treeNode) {
      sysDefStack.push_back(index);
      drawSysDefs(i.subDefs,accepted,sysDefStack);
      sysDefStack.erase(sysDefStack.end()-1);
      ImGui::TreePop();
    }
    if (isHovered) {
      if (ImGui::BeginTooltip()) {
        std::map<DivSystem,int> chipCounts;
        std::vector<DivSystem> chips;
        for (FurnaceGUISysDefChip chip: i.orig) {
          if (chipCounts.find(chip.sys)==chipCounts.end()) {
            chipCounts[chip.sys]=1;
            chips.push_back(chip.sys);
          } else {
            chipCounts[chip.sys]+=1;
          }
        }
        for (size_t chipIndex=0; chipIndex<chips.size(); chipIndex++) {
          DivSystem chip=chips[chipIndex];
          const DivSysDef* sysDef=e->getSystemDef(chip);
          ImGui::PushTextWrapPos(MIN(scrW*dpiScale,400.0f*dpiScale));
          ImGui::Text("%s (x%d): ",sysDef->name,chipCounts[chip]);
          ImGui::Text("%s",sysDef->description);
          ImGui::PopTextWrapPos();
          if (chipIndex+1<chips.size()) {
            ImGui::Separator();
          }
        }

        ImGui::EndTooltip();
      }
    }
    index++;
  }
}

void FurnaceGUI::drawNewSong() {
  bool accepted=false;
  std::vector<int> sysDefStack;

  ImGui::PushFont(bigFont);
  ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize("Choose a System!").x)*0.5);
  ImGui::Text("Choose a System!");
  ImGui::PopFont();

  ImVec2 avail=ImGui::GetContentRegionAvail();
  avail.y-=ImGui::GetFrameHeightWithSpacing();

  if (ImGui::BeginChild("sysPickerC",avail,false,ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar)) {
    if (newSongFirstFrame)
      ImGui::SetKeyboardFocusHere();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputTextWithHint("##SysSearch","Search...",&newSongQuery)) {
      String lowerCase=newSongQuery;
      for (char& i: lowerCase) {
        if (i>='A' && i<='Z') i+='a'-'A';
      }
      auto lastItem=std::remove_if(lowerCase.begin(),lowerCase.end(),[](char c) {
        return (c==' ' || c=='_' || c=='-');
      });
      lowerCase.erase(lastItem,lowerCase.end());
      newSongSearchResults.clear();
      for (FurnaceGUISysCategory& i: sysCategories) {
        for (FurnaceGUISysDef& j: i.systems) {
          String lowerCase1=j.name;
          for (char& i: lowerCase1) {
            if (i>='A' && i<='Z') i+='a'-'A';
          }
          auto lastItem=std::remove_if(lowerCase1.begin(),lowerCase1.end(),[](char c) {
            return (c==' ' || c=='_' || c=='-');
          });
          lowerCase1.erase(lastItem,lowerCase1.end());
          if (lowerCase1.find(lowerCase)!=String::npos) {
            newSongSearchResults.push_back(j);
          }
        }
        std::sort(newSongSearchResults.begin(),newSongSearchResults.end(),[](const FurnaceGUISysDef& a, const FurnaceGUISysDef& b) {
          return strcmp(a.name.c_str(),b.name.c_str())<0;
        });
        auto lastItem=std::unique(newSongSearchResults.begin(),newSongSearchResults.end(),[](const FurnaceGUISysDef& a, const FurnaceGUISysDef& b) {
          return a.name==b.name;
        });
        newSongSearchResults.erase(lastItem,newSongSearchResults.end());
      }
    }
    if (ImGui::BeginTable("sysPicker",newSongQuery.empty()?2:1,ImGuiTableFlags_BordersInnerV)) {
      if (newSongQuery.empty()) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0f);
      }
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

      if (newSongQuery.empty()) {
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::Text("Categories");
        ImGui::TableNextColumn();
        ImGui::Text("Systems");
      }

      ImGui::TableNextRow();

      // CATEGORIES
      if (newSongQuery.empty()) {
        ImGui::TableNextColumn();
        int index=0;
        for (FurnaceGUISysCategory& i: sysCategories) {
          if (ImGui::Selectable(i.name,newSongCategory==index,ImGuiSelectableFlags_DontClosePopups)) {
            newSongCategory=index;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s",i.description);
          }
          if (strcmp(i.name,"User")==0) ImGui::Separator();
          index++;
        }
      }

      // SYSTEMS
      ImGui::TableNextColumn();
      if (ImGui::BeginTable("Systems",1,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollY)) {
        std::vector<FurnaceGUISysDef>& category=(newSongQuery.empty())?(sysCategories[newSongCategory].systems):(newSongSearchResults);
        if (category.empty()) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          if (newSongQuery.empty()) {
            ImGui::Text("no systems here yet!");
          } else {
            ImGui::Text("no results");
          }
        } else {
          sysDefStack.push_back(newSongQuery.empty()?newSongCategory:-1);
          drawSysDefs(category,accepted,sysDefStack);
          sysDefStack.erase(sysDefStack.end()-1);
        }
        ImGui::EndTable();
      }

      ImGui::EndTable();
    }
  }
  ImGui::EndChild();

  if (ImGui::Button("I'm feeling lucky")) {
    if (sysCategories.size()==0) {
      showError("no categories available! what in the world.");
      ImGui::CloseCurrentPopup();
    } else {
      int tries=0;
      for (tries=0; tries<50; tries++) {
        FurnaceGUISysCategory* newSystemCat=&sysCategories[rand()%sysCategories.size()];
        if (newSystemCat->systems.empty()) {
          continue;
        } else {
          unsigned int selection=rand()%newSystemCat->systems.size();

          if (newSystemCat->systems[selection].orig.empty() && newSystemCat->systems[selection].subDefs.empty()) continue;
          if (!newSystemCat->systems[selection].subDefs.empty()) {
            if (rand()%2) {
              unsigned int subSel=rand()%newSystemCat->systems[selection].subDefs.size();
              nextDesc=newSystemCat->systems[selection].subDefs[subSel].definition;
              nextDescName=newSystemCat->systems[selection].subDefs[subSel].name;
              accepted=true;
            } else {
              if (newSystemCat->systems[selection].orig.empty()) continue;
              nextDesc=newSystemCat->systems[selection].definition;
              nextDescName=newSystemCat->systems[selection].name;
              accepted=true;
            }
          } else {
            nextDesc=newSystemCat->systems[selection].definition;
            nextDescName=newSystemCat->systems[selection].name;
            accepted=true;
          }
        }

        if (accepted) break;
      }

      if (tries>=50) {
        showError("it appears you're extremely lucky today!");
        ImGui::CloseCurrentPopup();
      }
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    ImGui::CloseCurrentPopup();
  }

  if (accepted) {
    e->createNew(nextDesc.c_str(),nextDescName,false);
    undoHist.clear();
    redoHist.clear();
    curFileName="";
    modified=false;
    curNibble=false;
    orderNibble=false;
    orderCursor=-1;
    samplePos=0;
    updateSampleTex=true;
    selStart=SelectionPoint();
    selEnd=SelectionPoint();
    cursor=SelectionPoint();
    updateWindowTitle();
    ImGui::CloseCurrentPopup();
  }

  newSongFirstFrame=false;
}
