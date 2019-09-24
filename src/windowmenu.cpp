/*
  nanogui/windowmenu.h -- Standard window menu with support for submenus

  NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
  The widget drawing code is based on the NanoVG demo application
  by Mikko Mononen.

  WindowMenu was developed by dalerank <dalerankn8@gmail.com>

  All rights reserved. Use of this source code is governed by a
  BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/windowmenu.h>
#include <nanogui/label.h>
#include <nanogui/entypo.h>
#include <nanogui/opengl.h>
#include <nanogui/layout.h>
#include <nanogui/screen.h>
#include <nanovg.h>

NAMESPACE_BEGIN(nanogui)

WindowMenu::WindowMenu(Widget *parent)
    : ContextMenu(parent, false)
{
  activate({ 0, 0 });
  mItemMargin = 2;
  mItemHLayout = new BoxLayout(Orientation::Horizontal);
  mItemContainer->setLayout(mItemHLayout);
}

Vector2i WindowMenu::preferredSize(NVGcontext* ctx) const {
  float bounds[4];
  nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
  nvgTextBounds(ctx, 0, 0, "A", nullptr, bounds);
  return Vector2i( parent()->width(), bounds[3] - bounds[1] + mItemMargin * 2);
}

void WindowMenu::addItem(const std::string& name, const std::function<void()>& value, int icon) 
{
  mItems[name] = value;
  auto lbl = new Label(mItemContainer, name);
  mLabels[name] = lbl;
  int prefh = preferredSize(screen()->nvgContext()).y();

  lbl->setFontSize(fontSize());
  int tw = nvgTextBounds(screen()->nvgContext(), 0, 0, name.c_str(), nullptr, nullptr);
  lbl->setPosition(mItemSpacing, mItemMargin);
  lbl->setTextHAlign(Label::TextHAlign::hCenter);
  lbl->setSize(tw + mItemSpacing*2, prefh);
  lbl->setFixedSize({ tw + mItemSpacing * 2, prefh });
  
  if (nvgIsFontIcon(icon)) {
    auto iconLbl = new Label(mItemContainer, utf8(icon).data(), "icons");
    iconLbl->setFontSize(fontSize() + 2);
  }
}

ContextMenu* WindowMenu::addSubMenu(const std::string& name, int icon) 
{
  if (!mParent)
    return nullptr;

  int prefh = preferredSize(screen()->nvgContext()).y();

  mSubmenus[name] = new ContextMenu(mParent, false);
  mSubmenus[name]->setRoot(mRootMenu ? mRootMenu : this);
  auto lbl = new Label(mItemContainer, name);
  mLabels[name] = lbl;
  lbl->setFontSize(fontSize());
  int tw = nvgTextBounds(screen()->nvgContext(), 0, 0, name.c_str(), nullptr, nullptr);
  lbl->setTextHAlign(Label::TextHAlign::hCenter);
  lbl->setSize(tw + mItemSpacing * 2, prefh);
  lbl->setFixedSize({ tw + mItemSpacing * 2, prefh });

  mSubmenus[name]->setMinWidth(lbl->width());

  if (nvgIsFontIcon(icon)) {
    auto iconLbl = new Label(mItemContainer, utf8(icon).data(), "icons");
    iconLbl->setFontSize(fontSize() + 2);
  }
  return mSubmenus[name];
}

void WindowMenu::deactivate() {
  if (mActivated) {
    mActivated = false;
    deactivateSubmenu();
  }
}

Vector2i WindowMenu::submenuPosition(const std::string &name) const {
  return mItemContainer->position() + mLabels.at(name)->position() + Vector2i{ 0, height() } + mPos;
}

bool WindowMenu::_isLabelSelected(const std::string& name, const Vector2i& p) const {
  if (mLabels.find(name) == mLabels.end())
    return false;
  auto w = mLabels.at(name);
  Vector4i r = w->rect();
  return p.y() >= r.y() && p.y() < r.w() && p.x() >= r.x() && p.x() < r.z();
}

bool WindowMenu::mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button, int modifiers) {
  Widget::mouseMotionEvent(p, rel, button, modifiers);
  Vector2i mousePos = p - mPos;
  for (const auto& w : mLabels) {
    // Deactivate old highlighted submenu, activate new submenu
    if (_isLabelSelected(w.first, mousePos)) {
      // Deactivate current submenu unless we are still hovering it.
      if (mActiveSubmenu && !(isSubMenu_(w.first) && mSubmenus[w.first] == mActiveSubmenu)) {
        deactivateSubmenu();
      }
      // Activate the item we are hovering
      if (isSubMenu_(w.first) && mSubmenus[w.first] != mActiveSubmenu) {
        bringToFront();
        activateSubmenu(w.first);
      }
      mHighlightedItem = mLabels[w.first];
      break;
    }
  }
  return true;
}

void WindowMenu::draw(NVGcontext* ctx) 
{
  nvgSave(ctx);
  nvgTranslate(ctx, 0, 0);

  int ww = width();
  int hh = height();

  /* Draw background */
  nvgBeginPath(ctx);
  nvgRect(ctx, 0, 0, ww, hh);
  nvgFillColor(ctx, mBackgroundColor);
  nvgFill(ctx);

  /* Draw margin background */
  if (!mLabels.empty()) 
  {
    auto lbl = mLabels.begin()->second;

    nvgBeginPath(ctx);
    nvgRect(ctx, 0, 0, ww - 1, hh);
    nvgFillColor(ctx, mMarginColor);
    nvgFill(ctx);
  }

  /* Draw outline */
  nvgBeginPath(ctx);
  nvgStrokeWidth(ctx, 1.0f);
  nvgRect(ctx, 0.5f, 1.5f, ww - 1, hh - 2);
  nvgStrokeColor(ctx, mTheme->mBorderLight);
  nvgStroke(ctx);

  nvgBeginPath(ctx);
  nvgRect(ctx, 0.5f, 0.5f, ww - 1, hh - 0.5f);
  nvgStrokeColor(ctx, mTheme->mBorderDark);
  nvgStroke(ctx);

  if (mHighlightedItem) 
  {
    nvgBeginPath(ctx);
    Vector4i r = mHighlightedItem->rect();
    nvgRect(ctx, r.x(), r.y(), r.z() - r.x(), r.w() - r.y() + mItemMargin *2);
    nvgFillColor(ctx, mHighlightColor);
    nvgFill(ctx);
  }
  nvgRestore(ctx);

  Widget::draw(ctx);
}

NAMESPACE_END(nanogui)