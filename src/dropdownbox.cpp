/*
    src/dropdownbox.cpp -- simple dropdown box widget based on a popup button

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/dropdownbox.h>
#include <nanogui/layout.h>
#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>
#include <algorithm>
#include <cassert>

NAMESPACE_BEGIN(nanogui)

class DropdownListItem : public Button
{
public:
  bool mInlist = true;

  DropdownListItem(Widget* parent, const std::string& str, bool inlist=true)
    : Button(parent, str), mInlist(inlist) {}

  void draw(NVGcontext *ctx) override 
  {
    if (!mInlist)
    {
      NVGcolor gradTop = mTheme->mButtonGradientTopPushed;
      NVGcolor gradBot = mTheme->mButtonGradientBotPushed;

      nvgBeginPath(ctx);

      nvgRoundedRect(ctx, mPos.x() + 1, mPos.y() + 1.0f, mSize.x() - 2,
        mSize.y() - 2, mTheme->mButtonCornerRadius - 1);

      if (mBackgroundColor.w() != 0) {
        nvgFillColor(ctx, Color(mBackgroundColor.head<3>(), 1.f));
        nvgFill(ctx);
        gradTop.a = gradBot.a = 0.8f;
      }

      NVGpaint bg = nvgLinearGradient(ctx, mPos.x(), mPos.y(), mPos.x(),
        mPos.y() + mSize.y(), gradTop, gradBot);

      nvgFillPaint(ctx, bg);
      nvgFill(ctx);

      nvgBeginPath(ctx);
      nvgStrokeWidth(ctx, 1.0f);
      nvgRoundedRect(ctx, mPos.x() + 0.5f, mPos.y() + 0.5f, mSize.x() - 1, mSize.y(), mTheme->mButtonCornerRadius);
      nvgStrokeColor(ctx, mTheme->mBorderLight);
      nvgStroke(ctx);

      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, mPos.x() + 0.5f, mPos.y() + 0.5f, mSize.x() - 1, mSize.y(), mTheme->mButtonCornerRadius);
      nvgStrokeColor(ctx, mTheme->mBorderDark);
      nvgStroke(ctx);
    }
    else
    {
      if (mMouseFocus && mEnabled) {
        NVGcolor gradTop = mTheme->mButtonGradientTopFocused;
        NVGcolor gradBot = mTheme->mButtonGradientBotFocused;

        nvgBeginPath(ctx);

        nvgRoundedRect(ctx, mPos.x() + 1, mPos.y() + 1.0f, mSize.x() - 2,
          mSize.y() - 2, mTheme->mButtonCornerRadius - 1);

        if (mBackgroundColor.w() != 0) {
          nvgFillColor(ctx, Color(mBackgroundColor.head<3>(), 1.f));
          nvgFill(ctx);
          if (mPushed) {
            gradTop.a = gradBot.a = 0.8f;
          }
          else {
            double v = 1 - mBackgroundColor.w();
            gradTop.a = gradBot.a = mEnabled ? v : v * .5f + .5f;
          }
        }

        NVGpaint bg = nvgLinearGradient(ctx, mPos.x(), mPos.y(), mPos.x(),
          mPos.y() + mSize.y(), gradTop, gradBot);

        nvgFillPaint(ctx, bg);
        nvgFill(ctx);
      }
    }

    NVGcolor textColor = mTextColor.w() == 0 ? mTheme->mTextColor : mTextColor;
    if (mPushed && mInlist)
    {
      Vector2f center = mPos.cast<float>() + mSize.cast<float>() * 0.5f;
      nvgBeginPath(ctx);
      nvgCircle(ctx, width() * 0.05f, center.y(), 2);
      nvgFillColor(ctx, textColor);
      nvgFill(ctx);
    }

    int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
    nvgFontSize(ctx, fontSize);
    nvgFontFace(ctx, "sans-bold");
    float tw = nvgTextBounds(ctx, 0, 0, mCaption.c_str(), nullptr, nullptr);

    Vector2f center = mPos.cast<float>() + mSize.cast<float>() * 0.5f;
    Vector2f textPos(center.x() - tw * 0.5f, center.y() - 1);
    if (!mEnabled)
      textColor = mTheme->mDisabledTextColor;

    if (mIcon) {
      auto icon = utf8(mIcon);

      float iw, ih = fontSize;
      if (nvgIsFontIcon(mIcon)) {
        ih *= icon_scale();
        nvgFontSize(ctx, ih);
        nvgFontFace(ctx, "icons");
        iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
      }
      else {
        int w, h;
        ih *= 0.9f;
        nvgImageSize(ctx, mIcon, &w, &h);
        iw = w * ih / h;
      }
      if (mCaption != "")
        iw += mSize.y() * 0.15f;
      nvgFillColor(ctx, textColor);
      nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
      Vector2f iconPos = center;
      iconPos.y() -= 1;

      if (mIconPosition == IconPosition::LeftCentered) {
        iconPos.x() -= (tw + iw) * 0.5f;
        textPos.x() += iw * 0.5f;
      }
      else if (mIconPosition == IconPosition::RightCentered) {
        textPos.x() -= iw * 0.5f;
        iconPos.x() += tw * 0.5f;
      }
      else if (mIconPosition == IconPosition::Left) {
        iconPos.x() = mPos.x() + 8;
      }
      else if (mIconPosition == IconPosition::Right) {
        iconPos.x() = mPos.x() + mSize.x() - iw - 8;
      }

      if (nvgIsFontIcon(mIcon)) {
        nvgText(ctx, iconPos.x(), iconPos.y() + 1, icon.data(), nullptr);
      }
      else {
        NVGpaint imgPaint = nvgImagePattern(ctx,
          iconPos.x(), iconPos.y() - ih / 2, iw, ih, 0, mIcon, mEnabled ? 0.5f : 0.25f);

        nvgFillPaint(ctx, imgPaint);
        nvgFill(ctx);
      }
    }

    nvgFontSize(ctx, fontSize);
    nvgFontFace(ctx, "sans-bold");
    nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgFillColor(ctx, mTheme->mTextColorShadow);
    nvgText(ctx, textPos.x(), textPos.y(), mCaption.c_str(), nullptr);
    nvgFillColor(ctx, textColor);
    nvgText(ctx, textPos.x(), textPos.y() + 1, mCaption.c_str(), nullptr);
    
    //Widget::draw(ctx);
  }
};

class DropdownPopup : public Popup
{
public:
  int preferredWidth = 0;

  DropdownPopup(Widget *parent, Window *parentWindow)
    : Popup(parent, parentWindow)
  {}

  float targetPath = 0;
  void hide() { targetPath = 0; }

  Vector2i preferredSize(NVGcontext *ctx) const override
  {
    Vector2i result = Popup::preferredSize(ctx);
    result.x() = preferredWidth;
    return result;
  }

  void refreshRelativePlacement() override
  {
    Popup::refreshRelativePlacement();
    mVisible &= mParentWindow->visibleRecursive();
    mPos = mParentWindow->position() + mAnchorPos;
  }

  void updateCaption(const std::string& caption)
  {
    if (mChildren.size() > 0)
    {
      auto* btn = dynamic_cast<Button*>(mChildren[0]);
      btn->setCaption(caption);
    }
  }

  void updateVisible(bool visible)
  {
    if (!visible)
    {
      if (path > 0) path -= 0.15f;
      if (path <= 0) path = 0.f;
    }
    else
    {
      if (path < 1.f) path += 0.15f;
      if (path > 1.f) path = 1.f;
    }

    mVisible = path > 0;
  }

  float path = 0.f;
  int clamp(int val, int min, int max) { return val < min ? min : (val > max ? max : val); }

  void draw(NVGcontext* ctx) override
  {
    refreshRelativePlacement();

    if (!mVisible || mChildren.empty())
      return;

    int ds = 1, cr = mTheme->mWindowCornerRadius;
    int ww = mFixedSize.x() > 0 ? mFixedSize.x() : mSize.x();

    int headerH = mChildren[0]->height();
    int realH = clamp(mSize.y() * path, headerH, mSize.y());

    nvgSave(ctx);
    nvgResetScissor(ctx);

    nvgIntersectScissor(ctx, mPos.x()-2, mPos.y() - 2, mSize.x() + 4, realH + 4);

    /* Draw a drop shadow */
    NVGpaint shadowPaint = nvgBoxGradient(
      ctx, mPos.x(), mPos.y(), ww, mSize.y(), cr * 2, ds * 2,
      mTheme->mDropShadow, mTheme->mTransparent);

    nvgBeginPath(ctx);
    nvgRect(ctx, mPos.x() - ds, mPos.y() - ds, ww + 2 * ds, mSize.y() + 2 * ds);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), ww, mSize.y(), cr);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, shadowPaint);
    nvgFill(ctx);

    /* Draw window */
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, mPos.x(), mPos.y(), ww, mSize.y(), cr);

    nvgFillColor(ctx, mTheme->mWindowPopup);
    nvgFill(ctx);

    if (mChildren.size() > 1)
    {
      nvgBeginPath(ctx);

      Vector2i fp = mPos + mChildren[1]->position();
      NVGpaint bg = nvgLinearGradient(ctx, fp.x(), fp.y(), fp.x(), fp.y() + 12 ,
                                      mTheme->mBorderMedium, mTheme->mTransparent);
      nvgRect(ctx, fp.x(), fp.y(), ww, 12);
      nvgFillPaint(ctx, bg);
      nvgFill(ctx);
    }

    Widget::draw(ctx);
    nvgRestore(ctx);
  }
};

DropdownBox::DropdownBox(Widget *parent) 
  : PopupButton(parent)
{
  mSelectedIndex = 0;
  Window *parentWindow = window();
  parentWindow->parent()->removeChild(mPopup);

  mPopup = new DropdownPopup(parentWindow->parent(), window());
  mPopup->setSize(Vector2i(320, 250));
  mPopup->setVisible(false);
  mPopup->setAnchorPos(Vector2i(0, 0));
}

DropdownBox::DropdownBox(Widget *parent, const std::vector<std::string> &items)
    : DropdownBox(parent) {
  setItems(items);
}

DropdownBox::DropdownBox(Widget *parent, const std::vector<std::string> &items, const std::vector<std::string> &itemsShort)
    : DropdownBox(parent) {
  setItems(items, itemsShort);
}

void DropdownBox::performLayout(NVGcontext *ctx) {
  PopupButton::performLayout(ctx);

  auto* dpopup = dynamic_cast<DropdownPopup*>(mPopup);
  if (dpopup)
  {
    dpopup->setAnchorPos(position());
    dpopup->preferredWidth = width();
  }
}

void DropdownBox::setSelectedIndex(int idx) {
    if (mItemsShort.empty())
        return;
    const std::vector<Widget *> &children = popup()->children();
    ((Button *) children[mSelectedIndex])->setPushed(false);
    ((Button *) children[idx])->setPushed(true);
    mSelectedIndex = idx;
    setCaption(mItemsShort[idx]);
    ((DropdownPopup*)mPopup)->updateCaption(mItemsShort[idx]);
}

void DropdownBox::setItems(const std::vector<std::string> &items, const std::vector<std::string> &itemsShort) {
    assert(items.size() == itemsShort.size());
    mItems = items;
    mItemsShort = itemsShort;
    if (mSelectedIndex < 0 || mSelectedIndex >= (int) items.size())
        mSelectedIndex = 0;
    
    while (mPopup->childCount() != 0)
      mPopup->removeChild(mPopup->childCount() - 1);
   
    mPopup->setLayout(new GroupLayout(0,0,0,0));
    if (!items.empty())
    {
      DropdownListItem *button = new DropdownListItem(mPopup, items[mSelectedIndex], false);
      button->setPushed(false);
      button->setCallback([&] { setPushed(false); popup()->setVisible(false); });
    }

    int index = 0;
    for (const auto &str: items) {
        DropdownListItem *button = new DropdownListItem(mPopup, str);
        button->setFlags(Button::RadioButton);
        button->setCallback([&, index] {
            mSelectedIndex = index;
            setCaption(mItemsShort[index]);
            setPushed(false);
            if (mCallback)
                mCallback(index);
        });
        index++;
    }
    setSelectedIndex(mSelectedIndex);
}

bool DropdownBox::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) 
{
  if (button == GLFW_MOUSE_BUTTON_1 && mEnabled) {
    if (!mItems.empty())
    {
      auto* item = dynamic_cast<DropdownListItem*>(mPopup->childAt(0));
      if (item)
        item->setCaption(mItems[mSelectedIndex]);
    }
  }

  return PopupButton::mouseButtonEvent(p, button, down, modifiers);
}

bool DropdownBox::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    if (rel.y() < 0) {
        setSelectedIndex(std::min(mSelectedIndex+1, (int)(items().size()-1)));
        if (mCallback)
            mCallback(mSelectedIndex);
        return true;
    } else if (rel.y() > 0) {
        setSelectedIndex(std::max(mSelectedIndex-1, 0));
        if (mCallback)
            mCallback(mSelectedIndex);
        return true;
    }
    return PopupButton::scrollEvent(p, rel);
}

void DropdownBox::draw(NVGcontext* ctx) {
  if (!mEnabled && mPushed)
    mPushed = false;

  ((DropdownPopup*)mPopup)->updateVisible(mPushed);
  Button::draw(ctx);

  if (mChevronIcon) {
    auto icon = utf8(mChevronIcon);
    NVGcolor textColor =
      mTextColor.w() == 0 ? mTheme->mTextColor : mTextColor;

    nvgFontSize(ctx, (mFontSize < 0 ? mTheme->mButtonFontSize : mFontSize) * icon_scale());
    nvgFontFace(ctx, "icons");
    nvgFillColor(ctx, mEnabled ? textColor : mTheme->mDisabledTextColor);
    nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

    float iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
    Vector2f iconPos(0, mPos.y() + mSize.y() * 0.5f - 1);

    if (mPopup->side() == Popup::Right)
      iconPos[0] = mPos.x() + mSize.x() - iw - 8;
    else
      iconPos[0] = mPos.x() + 8;

    nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr);
  }
}

void DropdownBox::save(Serializer &s) const {
    Widget::save(s);
    s.set("items", mItems);
    s.set("itemsShort", mItemsShort);
    s.set("selectedIndex", mSelectedIndex);
}

bool DropdownBox::load(Serializer &s) {
    if (!Widget::load(s)) return false;
    if (!s.get("items", mItems)) return false;
    if (!s.get("itemsShort", mItemsShort)) return false;
    if (!s.get("selectedIndex", mSelectedIndex)) return false;
    return true;
}

NAMESPACE_END(nanogui)
