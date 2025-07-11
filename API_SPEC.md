# PHP-EXT-LLHTTP API 仕様

## 概要

この仕様書は、php-ext-llhttp の独自 API 設計を定義します。シンプルで直感的な API を提供し、llhttp の高性能を PHP で活用できるようにします。

## 基本原則

### 1. シンプリシティ優先
- 複雑なイベントシステムは使用しない
- メソッド名は分かりやすく、一般的な PHP パターンに従う
- 最小限の設定で最大限の機能を提供

### 2. パフォーマンス重視
- ゼロコピー操作を可能な限り活用
- メモリ効率的なデータ管理
- 不要な PHP オブジェクト生成を避ける

### 3. 型安全性
- PHP 8.0+ の型宣言を活用
- 適切な戻り値型とパラメータ型
- null 安全性を考慮

## クラス構造

### Parser クラス

```php
<?php

namespace Llhttp;

class Parser
{
    // パーサータイプ定数
    public const TYPE_REQUEST = 0;
    public const TYPE_RESPONSE = 1;
    
    // HTTPメソッド定数
    public const METHOD_GET = 1;
    public const METHOD_POST = 2;
    public const METHOD_PUT = 3;
    public const METHOD_DELETE = 4;
    public const METHOD_HEAD = 5;
    public const METHOD_OPTIONS = 6;
    public const METHOD_PATCH = 7;
    // ... その他のメソッド
    
    /**
     * パーサーを初期化
     * 
     * @param int $type TYPE_REQUEST または TYPE_RESPONSE
     */
    public function __construct(int $type);
    
    /**
     * HTTPデータを解析
     * 
     * @param string $data 解析するHTTPデータ
     * @throws ParseException 解析エラー時
     */
    public function parse(string $data): void;
    
    /**
     * 解析完了を通知（必要に応じて）
     * 
     * @throws ParseException 不完全なデータの場合
     */
    public function parseComplete(): void;
    
    /**
     * パーサー状態をリセット
     */
    public function reset(): void;
    
    // === データアクセスメソッド ===
    
    /**
     * HTTPメジャーバージョン取得
     */
    public function getHttpMajor(): int;
    
    /**
     * HTTPマイナーバージョン取得
     */
    public function getHttpMinor(): int;
    
    /**
     * HTTPメソッド取得（リクエスト用）
     */
    public function getMethod(): int;
    
    /**
     * HTTPメソッド名取得（リクエスト用）
     */
    public function getMethodName(): string;
    
    /**
     * ステータスコード取得（レスポンス用）
     */
    public function getStatusCode(): int;
    
    /**
     * URL取得（リクエスト用）
     */
    public function getUrl(): string;
    
    /**
     * 全ヘッダーを配列で取得
     * 
     * @return array<string, string|array<string>> キー：ヘッダー名、値：ヘッダー値（複数値の場合は配列）
     */
    public function getHeaders(): array;
    
    /**
     * 指定ヘッダーの値を取得
     * 
     * @param string $name ヘッダー名（大文字小文字不区別）
     * @return string|null ヘッダー値（存在しない場合はnull）
     */
    public function getHeader(string $name): ?string;
    
    /**
     * HTTPボディを取得
     */
    public function getBody(): string;
    
    /**
     * Keep-Alive確認
     */
    public function shouldKeepAlive(): bool;
    
    /**
     * メッセージ完了にEOFが必要かどうか
     */
    public function messageNeedsEof(): bool;
    
    /**
     * 解析が完了したかどうか
     */
    public function isComplete(): bool;
    
    /**
     * 現在の解析状態を取得
     */
    public function getState(): int;
}
```

### 例外クラス

```php
<?php

namespace Llhttp;

/**
 * 解析エラー例外
 */
class ParseException extends \Exception
{
    private int $errorCode;
    private int $position;
    
    public function __construct(
        string $message,
        int $errorCode,
        int $position = 0,
        ?\Throwable $previous = null
    );
    
    /**
     * llhttp エラーコード取得
     */
    public function getErrorCode(): int;
    
    /**
     * エラー発生位置取得
     */
    public function getPosition(): int;
}
```

### ErrorCodes クラス

```php
<?php

namespace Llhttp;

/**
 * llhttp エラーコード定数
 */
class ErrorCodes
{
    public const HPE_OK = 0;
    public const HPE_INTERNAL = 1;
    public const HPE_STRICT = 2;
    public const HPE_LF_EXPECTED = 3;
    public const HPE_UNEXPECTED_CONTENT_LENGTH = 4;
    // ... その他のエラーコード
    
    /**
     * エラーコードから説明文を取得
     */
    public static function getDescription(int $code): string;
}
```

## 使用例

### HTTPリクエストの解析

```php
<?php

use Llhttp\Parser;

$parser = new Parser(Parser::TYPE_REQUEST);

$httpData = "GET /path?query=value HTTP/1.1\r\n" .
           "Host: example.com\r\n" .
           "User-Agent: MyApp/1.0\r\n" .
           "\r\n";

$parser->parse($httpData);
$parser->parseComplete();

echo "Method: " . $parser->getMethodName() . "\n";
echo "URL: " . $parser->getUrl() . "\n";
echo "Host: " . $parser->getHeader('Host') . "\n";

var_dump($parser->getHeaders());
```

### HTTPレスポンスの解析

```php
<?php

use Llhttp\Parser;

$parser = new Parser(Parser::TYPE_RESPONSE);

$httpData = "HTTP/1.1 200 OK\r\n" .
           "Content-Type: application/json\r\n" .
           "Content-Length: 13\r\n" .
           "\r\n" .
           '{"ok": true}';

$parser->parse($httpData);

echo "Status: " . $parser->getStatusCode() . "\n";
echo "Content-Type: " . $parser->getHeader('Content-Type') . "\n";
echo "Body: " . $parser->getBody() . "\n";
```

### ストリーミング解析

```php
<?php

use Llhttp\Parser;

$parser = new Parser(Parser::TYPE_REQUEST);

// データを段階的に追加
$parser->parse("GET /path HTTP/1.1\r\n");
$parser->parse("Host: example.com\r\n");
$parser->parse("\r\n");

if ($parser->isComplete()) {
    echo "Parsing complete!\n";
    echo "Method: " . $parser->getMethodName() . "\n";
    echo "URL: " . $parser->getUrl() . "\n";
}
```

### エラーハンドリング

```php
<?php

use Llhttp\Parser;
use Llhttp\ParseException;
use Llhttp\ErrorCodes;

$parser = new Parser(Parser::TYPE_REQUEST);

try {
    $parser->parse("INVALID HTTP DATA\r\n");
} catch (ParseException $e) {
    echo "Parse error: " . $e->getMessage() . "\n";
    echo "Error code: " . $e->getErrorCode() . "\n";
    echo "Position: " . $e->getPosition() . "\n";
    echo "Description: " . ErrorCodes::getDescription($e->getErrorCode()) . "\n";
}
```

## パフォーマンス考慮事項

### メモリ効率
- ヘッダーやボディのデータは必要時まで文字列化を遅延
- 大きなボディデータに対するストリーミング処理サポート
- 不要なデータコピーを最小限に抑制

### CPU効率
- llhttp のゼロコピー特性を最大限活用
- PHP オブジェクトの生成を最小限に抑制
- 文字列操作の最適化

## 互換性

### PHP バージョン
- **最小要件**: PHP 8.0
- **推奨**: PHP 8.1 以上
- **テスト対象**: PHP 8.0, 8.1, 8.2, 8.3

### プラットフォーム
- Linux (x86_64, ARM64)
- macOS (Intel, Apple Silicon)
- Windows (x86_64) - 将来対応予定

## 設計決定の根拠

### シンプルな API
従来の複雑なイベントシステムに代わり、直接的なメソッド呼び出しパターンを採用。PHP 開発者にとって理解しやすく、デバッグが容易。

### 型安全性
PHP 8.0 以上を対象とすることで、現代的な型システムを活用。実行時エラーを防ぎ、IDE サポートを向上。

### パフォーマンス優先
llhttp の設計思想に合わせ、高速処理を最優先。メモリ効率と CPU 効率の両方を考慮。

### 拡張性
将来的な機能追加に対応できる柔軟な設計。後方互換性を保ちながら新機能を追加可能。