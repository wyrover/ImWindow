
#include "ImwContainer.h"

#include "ImwWindowManager.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <algorithm>

const int c_iTabHeight = 20;

using namespace ImWindow;

ImwContainer::ImwContainer(ImwContainer* pParent)
{
	m_pParent = pParent;
	m_pSplits[0] = NULL;
	m_pSplits[1] = NULL;
	m_bVerticalSplit = false;
	m_iActiveWindow = 0;
	m_fSplitRatio = 0.5f;
	m_bIsDrag = false;
}

ImwContainer::~ImwContainer()
{
	while ( m_lWindows.begin() != m_lWindows.end() )
	{
		ImwWindowManager::GetInstance()->RemoveWindow(*m_lWindows.begin());
		m_lWindows.erase(m_lWindows.begin());
	}

	ImwSafeDelete(m_pSplits[0]);
	ImwSafeDelete(m_pSplits[1]);
}

void ImwContainer::CreateSplits()
{
	m_pSplits[0] = new ImwContainer(this);
	m_pSplits[1] = new ImwContainer(this);
}

void ImwContainer::Dock(ImwWindow* pWindow, EDockOrientation eOrientation)
{
	ImwAssert(NULL != pWindow);

	if ( NULL != pWindow )
	{
		ImwAssert(eOrientation == E_DOCK_ORIENTATION_CENTER || m_lWindows.size() > 0);

		if ( !IsSplit() )
		{
			if (m_lWindows.size() == 0)
			{
				eOrientation = E_DOCK_ORIENTATION_CENTER;
			}

			switch (eOrientation)
			{
			case E_DOCK_ORIENTATION_CENTER:
				{
					m_lWindows.push_back(pWindow);
					m_iActiveWindow = m_lWindows.size() - 1;
				}
				break;
			case E_DOCK_ORIENTATION_TOP:
				{
					m_bVerticalSplit = true;
					CreateSplits();
					m_pSplits[0]->Dock(pWindow);
					m_pSplits[1]->m_lWindows.insert(m_pSplits[1]->m_lWindows.begin(), m_lWindows.begin(), m_lWindows.end());
					m_lWindows.clear();
				}
				break;
			case E_DOCK_ORIENTATION_LEFT:
				{
					m_bVerticalSplit = false;
					CreateSplits();
					m_pSplits[0]->Dock(pWindow);
					m_pSplits[1]->m_lWindows.insert(m_pSplits[1]->m_lWindows.begin(), m_lWindows.begin(), m_lWindows.end());
					m_lWindows.clear();
				}
				break;
			case E_DOCK_ORIENTATION_RIGHT:
				{
					m_bVerticalSplit = false;
					CreateSplits();
					m_pSplits[0]->m_lWindows.insert(m_pSplits[0]->m_lWindows.begin(), m_lWindows.begin(), m_lWindows.end());
					m_pSplits[1]->Dock(pWindow);
					m_lWindows.clear();
				}
				break;
			case E_DOCK_ORIENTATION_BOTTOM:
				{
					m_bVerticalSplit = true;
					CreateSplits();
					m_pSplits[0]->m_lWindows.insert(m_pSplits[0]->m_lWindows.begin(), m_lWindows.begin(), m_lWindows.end());
					m_pSplits[1]->Dock(pWindow);
					m_lWindows.clear();
				}
				break;
			}
		}
		else
		{
			switch (eOrientation)
			{
			case E_DOCK_ORIENTATION_CENTER:
				if (m_bVerticalSplit)
				{
					m_pSplits[0]->Dock(pWindow, E_DOCK_ORIENTATION_BOTTOM);
				}
				else
				{
					m_pSplits[0]->Dock(pWindow, E_DOCK_ORIENTATION_RIGHT);
				}
				break;
			case E_DOCK_ORIENTATION_TOP:
			case E_DOCK_ORIENTATION_LEFT:
				if (m_bVerticalSplit)
				{
					m_pSplits[0]->Dock(pWindow, eOrientation);
				}
				else
				{
					m_pSplits[0]->Dock(pWindow, eOrientation);
				}
				break;
			case E_DOCK_ORIENTATION_RIGHT:
			case E_DOCK_ORIENTATION_BOTTOM:
				if (m_bVerticalSplit)
				{
					m_pSplits[1]->Dock(pWindow, eOrientation);
				}
				else
				{
					m_pSplits[1]->Dock(pWindow, eOrientation);
				}
				break;
			}
		}
	}
}

bool ImwContainer::UnDock(ImwWindow* pWindow)
{
	if (std::find(m_lWindows.begin(), m_lWindows.end(), pWindow) != m_lWindows.end())
	{
		m_lWindows.remove(pWindow);
		if (m_iActiveWindow >= m_lWindows.size())
		{
			m_iActiveWindow = m_lWindows.size() - 1;
		}
		return true;
	}
	if (NULL != m_pSplits[0] && NULL != m_pSplits[1])
	{
		if ( m_pSplits[0]->UnDock(pWindow) )
		{
			if (m_pSplits[0]->IsEmpty())
			{
				if (m_pSplits[1]->IsSplit())
				{
					ImwContainer* pSplit = m_pSplits[1];
					m_bVerticalSplit = pSplit->m_bVerticalSplit;
					ImwSafeDelete(m_pSplits[0]);
					m_pSplits[0] = pSplit->m_pSplits[0];
					m_pSplits[1] = pSplit->m_pSplits[1];
					pSplit->m_pSplits[0] = NULL;
					pSplit->m_pSplits[1] = NULL;
					m_pSplits[0]->m_pParent = this;
					m_pSplits[1]->m_pParent = this;
					ImwSafeDelete(pSplit);
				}
				else
				{
					m_lWindows.insert(m_lWindows.end(), m_pSplits[1]->m_lWindows.begin(), m_pSplits[1]->m_lWindows.end());
					ImwSafeDelete(m_pSplits[0]);
					ImwSafeDelete(m_pSplits[1]);
				}
			}
			return true;
		}

		if (m_pSplits[1]->UnDock(pWindow))
		{
			if (m_pSplits[1]->IsEmpty())
			{
				if (m_pSplits[0]->IsSplit())
				{
					ImwContainer* pSplit = m_pSplits[0];
					m_bVerticalSplit = pSplit->m_bVerticalSplit;
					ImwSafeDelete(m_pSplits[1]);
					m_pSplits[0] = pSplit->m_pSplits[0];
					m_pSplits[1] = pSplit->m_pSplits[1];
					pSplit->m_pSplits[0] = NULL;
					pSplit->m_pSplits[1] = NULL;
					m_pSplits[0]->m_pParent = this;
					m_pSplits[1]->m_pParent = this;
					ImwSafeDelete(pSplit);
				}
				else
				{
					m_lWindows.insert(m_lWindows.end(), m_pSplits[0]->m_lWindows.begin(), m_pSplits[0]->m_lWindows.end());
					ImwSafeDelete(m_pSplits[0]);
					ImwSafeDelete(m_pSplits[1]);
				}
			}
			return true;
		}
	}

	return false;
}

bool ImwContainer::IsEmpty()
{
	//ImwAssert(IsSplit() != HasWindowTabbed());
	return !(IsSplit() || HasWindowTabbed());
}

bool ImwContainer::IsSplit()
{
	ImwAssert((NULL == m_pSplits[0]) == (NULL == m_pSplits[1]));
	return (NULL != m_pSplits[0] && NULL != m_pSplits[1]);
}

bool ImwContainer::HasWindowTabbed()
{
	return m_lWindows.size() > 0;
}

ImwContainer* ImwContainer::HasWindow( const ImwWindow* pWindow)
{
	if (std::find(m_lWindows.begin(), m_lWindows.end(), pWindow) != m_lWindows.end())
	{
		return this;
	}
	else
	{
		if (NULL != m_pSplits[0])
		{
			ImwContainer* pContainer = m_pSplits[0]->HasWindow(pWindow);
			if (NULL != pContainer)
			{
				return pContainer;
			}
		}
		if (NULL != m_pSplits[1])
		{
			ImwContainer* pContainer = m_pSplits[1]->HasWindow(pWindow);
			if (NULL != pContainer)
			{
				return pContainer;
			}
		}
	}
	return NULL;
}

void ImwContainer::Paint(/* int iX, int iY, int iWidth, int iHeight */)
{
	ImwWindowManager* pWindowManager = ImwWindowManager::GetInstance();
	ImGuiWindow* pWindow = ImGui::GetCurrentWindow();
	const ImGuiIO& oIO = ImGui::GetIO();
	const ImGuiStyle& oStyle = ImGui::GetStyle();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	const ImVec2 oPos = ImGui::GetWindowPos();
	const ImVec2 oSize = ImGui::GetWindowSize();
	const ImVec2 oMin = ImVec2(oPos.x + 1, oPos.y + 1);
	const ImVec2 oMax = ImVec2(oPos.x + oSize.x - 2, oPos.y + oSize.y - 2);

	const int iSeparatorHalfSize = 3;
	const int iSeparatorSize = iSeparatorHalfSize * 2;

	if (IsSplit())
	{
		
		//const ImVec2 oPosMin(iX, iY);
		//const ImVec2 oPosMax(iX + iWidth, iY + iHeight);

		//pDrawList->AddRect(oMin, oMax, ImColor(255, 0, 0, 255));

		const ImGuiID oSeparatorId = pWindow->GetID("Separator");

		if (m_bVerticalSplit)
		{
			float iFirstHeight = oSize.y * m_fSplitRatio - iSeparatorHalfSize - pWindow->WindowPadding.x;
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
			//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
			/*ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0,0));
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);*/
			

			ImGui::BeginChild("Split1", ImVec2(0, iFirstHeight), false, ImGuiWindowFlags_NoScrollbar);
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,4));
			m_pSplits[0]->Paint(/*iX, iY, iWidth, iFirstHeight*/);
			//ImGui::PopStyleVar(1);
			ImGui::EndChild();


			ImRect oSeparatorRect( 0, iFirstHeight, oSize.x, iFirstHeight + iSeparatorSize);
			ImGui::Button("",oSeparatorRect.GetSize());
			if (ImGui::IsItemActive())
			{
				if (!m_bIsDrag)
				{
					m_fDragSplitStart = m_fSplitRatio;
					m_bIsDrag = true;
				}
				m_fSplitRatio = m_fDragSplitStart + (oIO.MousePos.y - oIO.MouseClickedPos[0].y) / oSize.y;
				m_fSplitRatio = ImClamp( m_fSplitRatio, 0.05f, 0.95f );
			}
			else
			{
				m_bIsDrag = false;
			}

			ImGui::BeginChild("Split2", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,4));
			m_pSplits[1]->Paint(/*iX, iY + iFirstHeight, iWidth, iSecondHeight*/);
			//ImGui::PopStyleVar(1);
			ImGui::EndChild();

			//ImGui::PopStyleVar(1);
		}
		else
		{
			float iFirstWidth = oSize.x * m_fSplitRatio - iSeparatorHalfSize - pWindow->WindowPadding.y;
			ImGui::BeginChild("Split1", ImVec2(iFirstWidth, 0), false, ImGuiWindowFlags_NoScrollbar);
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,4));
			m_pSplits[0]->Paint();
			//ImGui::PopStyleVar(1);
			ImGui::EndChild();

			ImGui::SameLine();

			ImRect oSeparatorRect( iFirstWidth, 0, iFirstWidth + iSeparatorSize, oSize.y);
			ImGui::Button("",oSeparatorRect.GetSize());
			if (ImGui::IsItemActive())
			{
				if (!m_bIsDrag)
				{
					m_fDragSplitStart = m_fSplitRatio;
					m_bIsDrag = true;
				}
				
				m_fSplitRatio = m_fDragSplitStart + (oIO.MousePos.x - oIO.MouseClickedPos[0].x) / oSize.x;
				m_fSplitRatio = ImClamp( m_fSplitRatio, 0.05f, 0.95f );
			}
			else
			{
				m_bIsDrag = false;
			}

			ImGui::SameLine();

			ImGui::BeginChild("Split2", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,4));
			m_pSplits[1]->Paint();
			//ImGui::PopStyleVar(1);
			ImGui::EndChild();

			//ImGui::PopStyleVar(1);
		}
	}
	else if (HasWindowTabbed())
	{
		//Tabs
		int iIndex = 0;
		int iNewActive = m_iActiveWindow;
		int iSize = m_lWindows.size();
		for (ImwWindowList::iterator it = m_lWindows.begin(); it != m_lWindows.end(); ++it)
		{
			const ImVec2 oTextSize = ImGui::CalcTextSize( (*it)->GetTitle() );
			const ImVec2 oRectSize(oTextSize.x + 4, oTextSize.y+2);

			ImU32 oFrameColor;

			ImGui::PushID(iIndex);


			bool bSelected = iIndex == m_iActiveWindow;
			if (ImGui::Selectable((*it)->GetTitle(), &bSelected, 0, oRectSize))
			{
				iNewActive = iIndex;
			}
			if (iIndex < (iSize - 1))
			{
				ImGui::SameLine();
			}

			if (ImGui::IsItemActive())
			{
				if (ImGui::IsMouseDragging())
				{
					pWindowManager->StartDragWindow( *it );
				}
			}

			if (ImGui::BeginPopupContextItem("Test"))
			{
				if (ImGui::Selectable("Close"))
				{
					(*it)->Destroy();
				}
				if (ImGui::BeginMenu("Dock to"))
				{
					int iIndex = 0;

					if (pWindowManager->GetMainPlatformWindow()->GetContainer()->IsEmpty())
					{
						ImGui::PushID(0);
						if (ImGui::Selectable("Main")) pWindowManager->Dock((*it));
						ImGui::PopID();
						++iIndex;
					}
					const ImwWindowList& lWindows = pWindowManager->GetWindowList();
					for (ImwWindowList::const_iterator itWindow = lWindows.begin(); itWindow != lWindows.end(); ++itWindow)
					{
						if ((*it) != (*itWindow))
						{
							ImGui::PushID(iIndex);
							if (ImGui::BeginMenu((*itWindow)->GetTitle()))
							{
								bool bHovered = false;
								ImwPlatformWindow* pPlatformWindow = pWindowManager->GetWindowParent((*itWindow));
								
								ImVec2 oLastWinPos = (*itWindow)->GetLastPosition();
								ImVec2 oLastWinSize = (*itWindow)->GetLastSize();
								
								ImGui::PushID(0);
								if (ImGui::Selectable("Tab")) pWindowManager->DockWith((*it), (*itWindow), E_DOCK_ORIENTATION_CENTER);
								if (ImGui::IsItemHovered() && NULL != pPlatformWindow)
								{
									bHovered = true;
									pWindowManager->DrawWindowArea(pPlatformWindow, oLastWinPos, oLastWinSize, ImColor(0.f, 0.5f, 1.f, 0.5f));
								}
								ImGui::PopID();

								ImGui::PushID(1);
								if (ImGui::Selectable("Top")) pWindowManager->DockWith((*it), (*itWindow), E_DOCK_ORIENTATION_TOP);
								if (ImGui::IsItemHovered() && NULL != pPlatformWindow)
								{
									bHovered = true;
									pWindowManager->DrawWindowArea(pPlatformWindow, oLastWinPos, ImVec2(oLastWinSize.x, oLastWinSize.y / 2.f), ImColor(0.f, 0.5f, 1.f, 0.5f));
								}
								ImGui::PopID();

								ImGui::PushID(2);
								if (ImGui::Selectable("Left")) pWindowManager->DockWith((*it), (*itWindow), E_DOCK_ORIENTATION_LEFT);
								if (ImGui::IsItemHovered() && NULL != pPlatformWindow)
								{
									bHovered = true;
									pWindowManager->DrawWindowArea(pPlatformWindow, oLastWinPos, ImVec2(oLastWinSize.x / 2.f, oLastWinSize.y), ImColor(0.f, 0.5f, 1.f, 0.5f));
								}
								ImGui::PopID();

								ImGui::PushID(3);
								if (ImGui::Selectable("Right")) pWindowManager->DockWith((*it), (*itWindow), E_DOCK_ORIENTATION_RIGHT);
								if (ImGui::IsItemHovered() && NULL != pPlatformWindow)
								{
									bHovered = true;
									pWindowManager->DrawWindowArea(pPlatformWindow, ImVec2(oLastWinPos.x + oLastWinSize.x / 2.f, oLastWinPos.y), ImVec2(oLastWinSize.x / 2.f, oLastWinSize.y), ImColor(0.f, 0.5f, 1.f, 0.5f));
								}
								ImGui::PopID();

								ImGui::PushID(4);
								if (ImGui::Selectable("Bottom")) pWindowManager->DockWith((*it), (*itWindow), E_DOCK_ORIENTATION_BOTTOM);
								if (ImGui::IsItemHovered() && NULL != pPlatformWindow)
								{
									bHovered = true;
									pWindowManager->DrawWindowArea(pPlatformWindow, ImVec2(oLastWinPos.x, oLastWinPos.y + oLastWinSize.y / 2.f), ImVec2(oLastWinSize.x, oLastWinSize.y / 2.f), ImColor(0.f, 0.5f, 1.f, 0.5f));
								}
								ImGui::PopID();

								if (!bHovered)
								{
									if ( NULL != pPlatformWindow )
									{
										pWindowManager->DrawWindowArea(pPlatformWindow, oLastWinPos, oLastWinSize, ImColor(0.f, 0.5f, 1.f, 0.5f));
									}
								}

								ImGui::EndMenu();
							}
							ImGui::PopID();
						}
						++iIndex;
					}
					
					ImGui::EndMenu();
				}
				if (ImGui::Selectable("Float")) pWindowManager->Float((*it));
				ImGui::EndPopup();
			}

			ImGui::PopID();

			++iIndex;
		}
		m_iActiveWindow = iNewActive;

		ImwWindowList::iterator itActiveWindow = m_lWindows.begin();
		std::advance(itActiveWindow, m_iActiveWindow);
		
		//Draw active
		ImwAssert(itActiveWindow != m_lWindows.end());
		if (itActiveWindow != m_lWindows.end())
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, oStyle.WindowPadding);
			//ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(59, 59, 59, 255));
			ImGui::BeginChild((*itActiveWindow)->GetId(), ImVec2(0,0), false);
			

			ImVec2 oWinPos = ImGui::GetWindowPos();
			ImVec2 oWinSize = ImGui::GetWindowSize();

			for (ImwWindowList::iterator it = m_lWindows.begin(); it != m_lWindows.end(); ++it)
			{
				(*it)->m_oLastPosition = oWinPos;
				(*it)->m_oLastSize = oWinSize;
			}
			(*itActiveWindow)->OnGui();
			
			ImGui::EndChild();
			//ImGui::PopStyleColor(1);
			ImGui::PopStyleVar(1);
		}
	}
	else
	{
		// This case can happened only where it's main container
		ImwAssert(m_pParent == NULL);
	}
}