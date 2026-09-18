#include "juce_graphics/juce_graphics.h"
#include "MC500_InputComponent.h"

MC500_InputComponent::MC500_InputComponent()
{
    // 液晶画面を可視化
    addAndMakeVisible (lcdArea);

    // 1. スライダー設定
    alphaDialSlider.setLookAndFeel (&dialLookAndFeel);
    alphaDialSlider.setRange (0.0, 100.0);
    alphaDialSlider.setValue (50.0);
    addAndMakeVisible (alphaDialSlider);

    // 2. 左ボタン設定
    setupButton (tieButton, "TIE");
    setupButton (restButton, "REST");

    // 3. 中央ボタン設定（2列4行分 = 8個）
    std::vector<juce::String> centerLabels {
        "MIDI", "EDIT",
        "FUNC", "MICRO",
        "MODE", "AVAIL",
        "SHIFT", "SPACE"
    };
    for (const auto& label : centerLabels)
    {
        auto* btn = new juce::TextButton();
        setupButton (*btn, label);
        centerButtons.add (btn);
    }

    // 4. 右テンキーボタン設定 (9個)
    std::vector<juce::String> numLabels {
        "7", "8", "9",
        "4", "5", "6",
        "1", "2", "3"
    };
    for (const auto& label : numLabels)
    {
        auto* btn = new juce::TextButton();
        setupButton (*btn, label);
        numButtons.add (btn);
    }

    // 5. 0 と ENTER
    setupButton (zeroButton, "0");
    setupButton (enterButton, "ENTER");
}

MC500_InputComponent::~MC500_InputComponent()
{
    // ★ ヌルポインタをセットして LookAndFeel の参照を安全に外す（JUCEの決まり文句）
    alphaDialSlider.setLookAndFeel (nullptr);
}

void MC500_InputComponent::paint (juce::Graphics& g)
{
    // 背景色
    //g.fillAll (juce::Colour (0xFFDCD5C5));
    //auto defaultBgColor = findColour (juce::ResizableWindow::backgroundColourId);
    //g.fillAll (defaultBgColor);
    g.fillAll (juce::Colours::darkgrey);

    g.setFont (12.0f);
    //g.setColour (juce::Colours::black);
    g.setColour (juce::Colours::white);

    // alpha-DIAL のテキスト位置をスライダーの真上に固定
    auto dialBounds = alphaDialSlider.getBounds();
    g.drawText ("alpha-DIAL", dialBounds.getX(), dialBounds.getY() + 2,
                dialBounds.getWidth(), 12, juce::Justification::centred);
}

void MC500_InputComponent::resized()
{
    // 全体の描画エリア（外側から15マス減らす）
    auto totalArea = getLocalBounds().reduced (15);

    // ==============================================================================
    // ★ 全体を上下2段に分割（上部30%を液晶、残りをコントローラー群にする）
    // ==============================================================================
    auto lcdBounds = totalArea.removeFromTop (totalArea.getHeight() * 0.30f);
    lcdArea.setBounds (lcdBounds);

    totalArea.removeFromTop (15); // 液晶とボタン群の間のスキマ
    auto area = totalArea;       // コントローラーエリア

    // -------------------------------------------------------------
    // 以下、従来の3ブロック分割計算（コントローラーエリア内で実行）
    // -------------------------------------------------------------
    // 計算しやすいように、全体の幅を「左:中央:右 = 5:5:7」の割合で綺麗にカットします
    int totalUnit = 5 + 5 + 7; // = 17
    int singleWidth = area.getWidth() / totalUnit;

    auto leftArea   = area.removeFromLeft (singleWidth * 5);
    area.removeFromLeft (20); // セクション間の隙間 (gap)

    auto centerArea = area.removeFromLeft (singleWidth * 5);
    area.removeFromLeft (20); // セクション間の隙間 (gap)

    auto rightArea  = area; // 残りが右セクション

    // -------------------------------------------------------------
    // 1. 【左セクション】の配置 (直接配置)
    // -------------------------------------------------------------
    // 上70%をダイヤル、下30%をボタン行にする
    auto dialArea = leftArea.removeFromTop (leftArea.getWidth());
    alphaDialSlider.setBounds (dialArea.reduced(5)); // 少し小さくして丸を綺麗に見せる

    leftArea.removeFromTop (8); // 縦の隙間

    // TIE と REST ボタンを横並びに分割
    auto tieArea = leftArea.removeFromLeft (leftArea.getWidth() / 2).reduced(2, 0);
    auto restArea = leftArea.reduced(2, 0);
    tieButton.setBounds (tieArea);
    restButton.setBounds (restArea);

    // -------------------------------------------------------------
    // 2. 【中央セクション】の配置 (2列4行 Grid)
    // -------------------------------------------------------------
    juce::Grid centerGrid;
    centerGrid.templateColumns = { juce::Grid::TrackInfo (juce::Grid::Fr (1)), juce::Grid::TrackInfo (juce::Grid::Fr (1)) };
    for (int i = 0; i < 4; ++i)
        centerGrid.templateRows.add (juce::Grid::TrackInfo (juce::Grid::Fr (1)));
    centerGrid.columnGap = juce::Grid::Px (6);
    centerGrid.rowGap    = juce::Grid::Px (6);

    for (auto* btn : centerButtons)
        centerGrid.items.add (juce::GridItem (*btn));

    centerGrid.performLayout (centerArea);

    // -------------------------------------------------------------
    // 3. 【右セクション】の配置 (テンキー変則 Grid)
    // -------------------------------------------------------------
    juce::Grid rightGrid;
    rightGrid.templateColumns = {
        juce::Grid::TrackInfo (juce::Grid::Fr (1)),
        juce::Grid::TrackInfo (juce::Grid::Fr (1)),
        juce::Grid::TrackInfo (juce::Grid::Fr (1))
    };
    for (int i = 0; i < 4; ++i)
        rightGrid.templateRows.add (juce::Grid::TrackInfo (juce::Grid::Fr (1)));
    rightGrid.columnGap = juce::Grid::Px (6);
    rightGrid.rowGap    = juce::Grid::Px (6);

    // 7,8,9, 4,5,6, 1,2,3
    for (auto* btn : numButtons)
        rightGrid.items.add (juce::GridItem (*btn));

    // 0 と ENTER (ENTERは2マス結合)
    rightGrid.items.add (juce::GridItem (zeroButton));
    rightGrid.items.add (juce::GridItem (enterButton).withArea (4, 2, 5, 4));

    rightGrid.performLayout (rightArea);
}

void MC500_InputComponent::setupButton (juce::TextButton& btn, const juce::String& text)
{
    btn.setButtonText (text);
    btn.setConnectedEdges (0);
    addAndMakeVisible (btn); // ここで親コンポーネントへ確実に登録
}
