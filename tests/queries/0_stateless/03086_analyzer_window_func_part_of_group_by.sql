-- https://github.com/ClickHouse/ClickHouse/issues/57321
SELECT
    ver,
    max(ver) OVER () AS ver_max
FROM
(
    SELECT 1 AS ver
    UNION ALL
    SELECT 2 AS ver
)
GROUP BY ver
