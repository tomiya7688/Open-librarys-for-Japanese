"""公式リンクポータルの軽量な静的チェック（外部依存なし）。"""

from html.parser import HTMLParser
from pathlib import Path
import unittest
from urllib.parse import urlsplit


HTML = Path(__file__).with_name("index.html")
CATEGORIES = {"Python", "C++", "開発ツール"}


class PortalParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.cards = []
        self.filters = []
        self.links = []
        self.current_card = None
        self.external_assets = []
        self.has_search = False

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag == "article" and "site" in attrs.get("class", "").split():
            self.current_card = {"category": attrs.get("data-category"), "links": [], "checked": False}
            self.cards.append(self.current_card)
        elif tag == "article" and self.current_card:
            raise AssertionError("カードのネストは非対応")
        if tag == "button" and attrs.get("data-filter") is not None:
            self.filters.append(attrs)
        if tag == "input" and attrs.get("id") == "search":
            self.has_search = attrs.get("type") == "search"
        if tag == "a" and attrs.get("href", "").startswith("https://"):
            self.links.append(attrs)
            if self.current_card is not None:
                self.current_card["links"].append(attrs)
        if (tag == "script" and attrs.get("src")) or (
            tag == "link" and attrs.get("rel") == "stylesheet"
        ):
            self.external_assets.append(attrs)

    def handle_data(self, data):
        if self.current_card is not None and "URL確認日：2026-11-04" in data:
            self.current_card["checked"] = True

    def handle_endtag(self, tag):
        if tag == "article":
            self.current_card = None


class PortalTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.html = HTML.read_text(encoding="utf-8")
        cls.portal = PortalParser()
        cls.portal.feed(cls.html)

    def test_initial_cards_and_filters(self):
        self.assertEqual(len(self.portal.cards), 8)
        self.assertEqual(
            set(x["data-filter"] for x in self.portal.filters),
            {"all"} | CATEGORIES,
        )
        self.assertTrue(self.portal.has_search)
        self.assertIn('id="empty"', self.html)

    def test_official_links_have_safety_attributes(self):
        primary_urls = []
        for card in self.portal.cards:
            self.assertIn(card["category"], CATEGORIES)
            self.assertTrue(card["checked"])
            primary = [a for a in card["links"] if "primary" in a.get("class", "").split()]
            self.assertEqual(len(primary), 1)
            primary_urls.append(primary[0]["href"])

        self.assertEqual(len(set(primary_urls)), len(primary_urls))
        for link in self.portal.links:
            self.assertEqual(urlsplit(link["href"]).scheme, "https")
            self.assertEqual(link.get("target"), "_blank")
            self.assertIn("noopener", link.get("rel", "").split())
            self.assertIn("noreferrer", link.get("rel", "").split())

    def test_offline_readable_without_external_dependencies(self):
        self.assertEqual(self.portal.external_assets, [])
        self.assertIn('<html lang="ja">', self.html)
        self.assertIn("<noscript>", self.html)
        self.assertIn('<div class="sites" id="sites">', self.html)


if __name__ == "__main__":
    unittest.main()
