#include "moveGameLayer.hpp"

void moveGameLayerSmoothAbs(EditorUI* ui, CCPoint const& pos) {
    ui->m_editorLayer->getObjectLayer()->stopActionByTag(0xbb);
    auto a = CCEaseInOut::create(
        CCMoveTo::create(.4f, pos),
        2.0f
    );
    a->setTag(0xbb);
    ui->m_editorLayer->getObjectLayer()->runAction(a);
}

void moveGameLayerSmooth(EditorUI* ui, CCPoint const& pos) {
    auto opos = ui->m_editorLayer->getObjectLayer()->getPosition();
    auto npos = opos + pos;
    moveGameLayerSmoothAbs(ui, npos);
}

void focusGameLayerOnObject(EditorUI* ui, GameObject* obj) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto olayer = ui->m_editorLayer->getObjectLayer();
    auto npos = (-obj->getPosition() + winSize / 2) * olayer->getScale();

    moveGameLayerSmoothAbs(ui, npos);
}

void focusGameLayerToSelection(EditorUI* ui) {
    CCPoint pos;

    if (ui->m_selectedObject)
        pos = ui->m_selectedObject->getPosition();
    else if (ui->m_selectedObjects)
        pos = ui->getGroupCenter(ui->m_selectedObjects, false);
    else return;

    auto olayer = ui->m_editorLayer->getObjectLayer();
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto gpos = olayer->convertToWorldSpace(pos);
    auto npos = (-pos + winSize / 2) * olayer->getScale();
    auto opos = olayer->getPosition();

    auto mx = (
        gpos.x < w_edge ||
        gpos.x > winSize.width - w_edge
    );
    auto my = (
        gpos.y < w_edge ||
        gpos.y > winSize.height - w_edge
    );

    if (mx && my)
        moveGameLayerSmoothAbs(ui, { npos.x, npos.y });
    else if (mx)
        moveGameLayerSmoothAbs(ui, { npos.x, opos.y });
    else if (my)
        moveGameLayerSmoothAbs(ui, { opos.x, npos.y });
}

