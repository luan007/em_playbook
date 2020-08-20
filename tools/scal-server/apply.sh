cp static/json/data.json ../../server/static/lscal/news/data.json
cp static/json/data.json ../../server/static/scal/news/data.json

_scal_meta_head="
{
    \"version\": $RANDOM,
    \"cache\": 1000,
    \"showDate\": {
        \"m\": 7,
        \"d\": 21
    },
    \"render_alias\": {
        \"root.html\": [
            \"NEWS\"
        ]
    },
    \"capability\": {
        \"asset-pack\": 1
    }
}
"
echo $_scal_meta_head > ../../server/static/scal/news/meta.json


_lscal_meta_head="
{
    \"version\": $RANDOM,
    \"render_alias\": {
        \"root.html\": [
            \"NEWS\"
        ]
    },
    \"capability\": {
        \"asset-pack\": 1
    }
}
"
echo $_lscal_meta_head > ../../server/static/lscal/news/meta.json
