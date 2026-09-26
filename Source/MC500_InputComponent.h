#pragma once
#include <array>
#include <cstddef>
#include <juce_gui_basics/juce_gui_basics.h>
#include "InfiniteRotarySliderComponent.h"
#include "LcdComponent.h"
#include "Multi10KeyButton.h"
#include "juce_core/juce_core.h"

// ==============================================================================
// ★実機風の黒い丸型ダイヤルを描画するためのカスタムLook&Feelクラス
// ==============================================================================
class DialLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DialLookAndFeel() {}

    void setVisualAngle (float newAngle) { visualAngle = newAngle; }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        juce::ignoreUnused(sliderPosProportional, rotaryStartAngle, rotaryEndAngle);

        // 描画エリアの計算（正方形の中心をとる）
        auto radius = juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;

        // ==============================================================================
        // ★【修正】つまみ本体の色を、周りのボタンと同じ色（デフォルトグレー）にする
        // ==============================================================================
        auto buttonColor = slider.findColour (juce::TextButton::buttonColourId);
        g.setColour (buttonColor);
        g.fillEllipse (rx, ry, rw, rw);

        // 2. 外周の立体感を出す境界線（ベベル効果）
        // つまみが明るくなったので、影（黒）とハイライト（白）の不透明度を調整して馴染ませます
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.drawEllipse (rx, ry, rw, rw, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.drawEllipse (rx + 1.0f, ry + 1.0f, rw - 2.0f, rw - 2.0f, 1.0f);

        // 3. 回転角度の計算
        //auto currentAngle = rotaryStartAngle + (sliderPosProportional * (rotaryEndAngle - rotaryStartAngle));
        auto currentAngle = visualAngle;

        // 4. インジケーター（指標線）の描画
        // つまみがグレーになったため、指標線は「黒（または濃いグレー）」にすると見やすくなります
        juce::Path p;
        auto pointerLength = radius * 0.8f;
        auto pointerThickness = 3.0f;

        p.addRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, radius * 0.25f);
        p.applyTransform (juce::AffineTransform::rotation (currentAngle).translated (centreX, centreY));

        g.setColour(juce::Colour (0xFFECEAE4));
        //g.setColour (juce::Colour (0xFF2A2A2A)); // ★ 指標線を濃いグレーに変更して視認性を確保
        g.fillPath (p);
    }

private:
    float visualAngle = 0.0f; // ▽▽▽ 追記：見た目の回転角度を保持する変数 ▽▽▽
};

// ==============================================================================
// ★左右の矢印ボタン専用のカスタムLook&Feelクラス
// ==============================================================================
class ArrowButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ArrowButtonLookAndFeel (bool isLeftArrow) : isLeft (isLeftArrow) {}

    void setDrawMode(bool mode)
    {
        drawMode = mode;
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isMouseOverButton, bool isButtonDown) override
    {
        auto textColor = button.findColour (juce::TextButton::textColourOffId);
        if (isButtonDown)           g.setColour (textColor.withAlpha (0.6f));
        else if (isMouseOverButton) g.setColour (textColor.brighter (0.1f));
        else                        g.setColour (textColor);

        if (!drawMode)
        {
            auto width = (float) button.getWidth();
            auto height = (float) button.getHeight();

            juce::Path p;

            // ボタンの中心を基準に、綺麗な正三角形のサイズを決定
            auto arrowSize = juce::jmin (width, height) * 0.35f;
            auto centerX = width * 0.5f;
            auto centerY = height * 0.5f;

            if (isLeft)
            {
                // 左向きの三角形 (◀)
                p.addTriangle (centerX - arrowSize * 0.5f, centerY,                  // 先端（左）
                            centerX + arrowSize * 0.5f, centerY - arrowSize * 0.4f, // 右上
                            centerX + arrowSize * 0.5f, centerY + arrowSize * 0.4f); // 右下
            }
            else
            {
                // 右向きの三角形 (▶)
                p.addTriangle (centerX + arrowSize * 0.5f, centerY,                  // 先端（右）
                            centerX - arrowSize * 0.5f, centerY - arrowSize * 0.4f, // 左上
                            centerX - arrowSize * 0.5f, centerY + arrowSize * 0.4f); // 左下
            }
            g.fillPath (p);
        }
        else
        {
            auto bounds = button.getLocalBounds().toFloat();
            float fontSize = bounds.getHeight() * 0.36f;
            g.setFont (juce::Font (fontSize, juce::Font::plain));

            if (isLeft)
            {
                auto buttonText = juce::String("TIE");
                g.drawText (buttonText, bounds, juce::Justification::centred, true);
            }
            else
            {
                auto buttonText = juce::String("REST");
                g.drawText (buttonText, bounds, juce::Justification::centred, true);
            }
        }
    }

private:
    bool isLeft;
    bool drawMode = false;
};

// ==============================================================================
// メインコンポーネントクラス
// ==============================================================================
class MC500_InputComponent  : public juce::Component,
                              private juce::Timer
{
public:
    MC500_InputComponent();
    ~MC500_InputComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void visibilityChanged() override;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) override;

private:
    void timerCallback() override;

    // ダイヤルが現在どのパラメーターを操作しているかを表す状態定義
    enum class DialTargetMode
    {
        None,
        StepTime,
        Note,
        Velocity,
        GateTime
    };

    DialTargetMode currentDialMode = DialTargetMode::StepTime; // 初期状態はStepTime

    static constexpr std::array<int, 10> stepTimeValues
    {
        192, 96, 64, 48, 32,
         24, 16, 12,  8,  6
    };

    std::size_t stepTimeIndex = 1;   // 初期値 96

    // --- UI素材の定義 ---
    LcdComponent lcdArea;

    InfiniteRotarySliderComponent alphaDialSlider;
    float dialVisualAngle = 0.0f;
    juce::TextButton leftButton;
    juce::TextButton rightButton;

    // 中央のボタン配列 (OwnedArrayはスマートポインタの動的配列)
    juce::OwnedArray<juce::TextButton> centerButtons;

    // 右のテンキー配列
    juce::OwnedArray<Multi10KeyButton> numButtons;
    juce::TextButton enterButton;

    // ★カスタムLook&Feelのインスタンスを追加
    DialLookAndFeel dialLookAndFeel;
    ArrowButtonLookAndFeel leftArrowLookAndFeel { true };
    ArrowButtonLookAndFeel rightArrowLookAndFeel { false };

    void setupButton (juce::TextButton& btn, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MC500_InputComponent)
};
