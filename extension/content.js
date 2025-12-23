console.log("content.js loaded");
let imageId = null;

// ビーコンがあるかチェックする
const isBeaconExists = (element) => {
  // ## ビーコンサンプル
  // <image fill="#000000" width="231" height="215" preserveAspectRatio="none" transform="matrix(108.1299,0,0,108.1302,325350.8896,174820.8919)" x="0" y="0" xlink:href="blob:https://docs.google.com/e45ac1bf-3547-43af-af75-c3c60c4f93de"></image>

  // ビーコンが存在するかチェック
  // 属性名：xlink:href
  // 属性値：blob:https://docs.google.com/e45ac1bf-3547-43af-af75-c3c60c4f93de

  // 1スライド目の画像IDが取得できていなければ処理をしない
  if(imageId === null) {
    console.log("imageId is null");
    return false;
  }

  const beaconElement = element.querySelector(`[*|href*="${imageId}"]`);
  console.log(`beacon found: ${beaconElement !== null}`);

  return beaconElement !== null;
}

// 次のスライドへ
const nextSlide = (element) => {
  console.log("next slide");

  // 右矢印キーのイベントを発火させる
  const keyboardEvent = new KeyboardEvent("keydown", {
    key: "ArrowRight",
    code: "ArrowRight",
    keyCode: 39,
    which: 39,
    bubbles: true,
    cancelable: true
  });

  element.dispatchEvent(keyboardEvent);
}

// iframeにショートカットキーを設定する
// ※Chrome拡張機能にショートカットキーを設定できるらしく、そっちで実現すればよかったと後で後悔しました。とほほ。
const setKeyEvent = (iframeBody) => {
  iframeBody.addEventListener("keydown", (event) => {
    console.log(`iframe key: ${event.key}, ctrl: ${event.ctrlKey}`);

    // Pで次のスライドへ
    if (isBeaconExists(iframeBody) && event.key === 'm') {
      nextSlide(iframeBody);
    }
  });
  console.log("iframe key event added");

}

// 1スライド目の画像を取得するメイン
const setFirstSlideImageId = (iframeBody) => {
  // GoogleSlideのホバーメニューにスライド番号が載ってるのでホバーメニューを取得する
  const slideNavElement = iframeBody.querySelector(".docs-material-menu-button-flat-default-caption")
  // ホバーメニューのテキストからスライド番号を取得する
  const slideNumber = slideNavElement.textContent;
  console.log(`slide number: ${slideNumber}`);
  // スライド番号が1でなければ処理をしない
  if (slideNumber !== "1") return null;
  // スライドの画像要素を取得する
  const imageElement = iframeBody.querySelector('image');
  // 画像要素がなければ処理をしない
  if (!imageElement) return null;

  // 画像要素のxlink:href属性値を取得する
  const imageUrl = imageElement.getAttributeNS('http://www.w3.org/1999/xlink', 'href');
  console.log(`first slide imageId: ${imageId}`);

  // href属性値はURL形式なので、最後の/以降を取得する
  const imageIdParts = imageUrl.split('/');
  imageId = imageIdParts[imageIdParts.length - 1]; // グローバル変数にセットする
  console.log(`first slide imageId (final): ${imageId}`);

  return imageId;
}

// 1スライド目の画像を取得する
const firstSlideImageId = (iframeBody) => {
  // 最初に1スライド目の画像IDを取得する
  setFirstSlideImageId(iframeBody);

  // 切り替えるたびにチェックが走るようにする
  iframeBody.addEventListener("keydown", (event) => {
    setFirstSlideImageId(iframeBody);
  });
  console.log("first slide imageId event added");
}

// DOMの変更を検知する
const observer = new MutationObserver(records => {
  // 検知したDOMノードを調べる
  records.forEach(record => {
    // addedNodesで追加されたNodeを取得できる
    record.addedNodes.forEach(node => {
      // Nodeの変更でなければ処理をしない
      if(node.nodeType !== Node.ELEMENT_NODE) {
        return;
      }

      // googleSlideのフルスクリーンをつかさどるDOM要素を取得
      const iframeElement = node.querySelector(".punch-present-iframe");

      // iframeが見つからなければ処理をしない
      if (!iframeElement) {
        return;
      }

      console.log("iframe found");

      // 1秒待機してでDOMの読み込みを待つ
      setTimeout(() => {
        setKeyEvent(iframeElement.contentWindow.document.body);
        // GoogleSlideの画像は画面更新するたびにIDが変わる。
        // しょうがないのでタイトルページに画像を１つだけ挿入し、
        // 同じ画像があるスライドだけ次のスライドへ進むようにする。
        // そのため、1スライド目だったときにIDを取得する
        firstSlideImageId(iframeElement.contentWindow.document.body);
      }, 1000);
    });
  })
})

observer.observe(document.body, {
  childList: true
});
