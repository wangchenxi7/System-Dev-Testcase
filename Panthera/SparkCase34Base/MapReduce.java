package SparkCase34Base;

public class MapReduce {

	public MapReduce( RDD rdd, Expr expr, Filter filter ) {
		this( null, rdd, expr, filter );
	}
	
	public MapReduce( MapReduce mapReduce, Expr expr, Filter filter ) {
		this( mapReduce, null, expr, filter );
	}
	
	public RDD execute() {
		RDD result = new RDD( size() );
		
		for( MapReduceIter iter = makeIter(); iter.hasNext(); ) {
			result.add( iter.next() );
		}
		
		return result;
	}
	
	private MapReduceIter makeIter() {
		if ( mapReduce != null ) {
			return new MapReduceIter( filter, expr, mapReduce.makeIter() );
		} else {
			return new MapReduceIter( filter, expr, rdd.begin() );
		}
	}
	
	private int size() {
		if ( mapReduce != null ) {
			return mapReduce.size();
		} else {
			return rdd.size();
		}
	}
	private MapReduce( MapReduce mapReduce, RDD rdd, Expr expr, Filter filter ) {
		this.mapReduce = mapReduce;
		this.rdd = rdd;
		this.expr = expr;
		this.filter = filter;
		
		if ( expr == null ) {
			this.expr = new Identity();
		}
		if ( filter == null ) {
			this.filter = new Any();
		}
	}
	
	private MapReduce mapReduce;
	private RDD rdd;
	private Expr expr;
	private Filter filter;
}
